import argparse, glob, io, os, sys, time

COCO80 = ("person bicycle car motorcycle airplane bus train truck boat traffic_light fire_hydrant stop_sign "
          "parking_meter bench bird cat dog horse sheep cow elephant bear zebra giraffe backpack umbrella handbag "
          "tie suitcase frisbee skis snowboard sports_ball kite baseball_bat baseball_glove skateboard surfboard "
          "tennis_racket bottle wine_glass cup fork knife spoon bowl banana apple sandwich orange broccoli carrot "
          "hot_dog pizza donut cake chair couch potted_plant bed dining_table toilet tv laptop mouse remote keyboard "
          "cell_phone microwave oven toaster sink refrigerator book clock vase scissors teddy_bear hair_drier "
          "toothbrush").split()
COLORS = [(255, 64, 64), (64, 200, 64), (64, 128, 255), (255, 200, 0), (255, 64, 255), (0, 220, 220)]


def log(*a):
    print("[rover_cam]", *a, file=sys.stderr, flush=True)


def open_recorder(dirpath, keep):
    os.makedirs(dirpath, exist_ok=True)
    try:
        boot = open("/proc/sys/kernel/random/boot_id").read().strip()
    except OSError:
        boot = "unknown"
    idf, cur = os.path.join(dirpath, "boot.id"), os.path.join(dirpath, "current.mjpg")
    same = False
    try:
        same = os.path.exists(cur) and open(idf).read().strip() == boot
    except OSError:
        pass
    if not same:
        if os.path.exists(cur):
            if os.path.getsize(cur) >= 2 * 1024 * 1024:
                stamp = time.strftime("boot-%Y%m%d-%H%M%S", time.localtime(os.path.getmtime(cur)))
                dst, n = os.path.join(dirpath, stamp + ".mjpg"), 1
                while os.path.exists(dst):
                    n += 1
                    dst = os.path.join(dirpath, "%s-%d.mjpg" % (stamp, n))
                os.rename(cur, dst)
            else:
                os.remove(cur)
        with open(idf, "w") as f:
            f.write(boot)
        old = sorted(glob.glob(os.path.join(dirpath, "boot-*.mjpg")), key=os.path.getmtime)
        for f in old[:max(0, len(old) - keep)]:
            os.remove(f)
    return open(cur, "ab"), cur


def uptime_text():
    try:
        sec = int(float(open("/proc/uptime").read().split()[0]))
        return "up %d:%02d:%02d" % (sec // 3600, sec // 60 % 60, sec % 60)
    except OSError:
        return ""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--model", required=True, help="IMX500 network file (.rpk), e.g. imx500_network_yolo11n_pp.rpk")
    ap.add_argument("--width", type=int, default=480)
    ap.add_argument("--height", type=int, default=360)
    ap.add_argument("--fps", type=int, default=15)
    ap.add_argument("--quality", type=int, default=80)
    ap.add_argument("--hq-width", type=int, default=0, help="high-quality profile (used only while the mode file says 'home'): width; 0 = off")
    ap.add_argument("--hq-height", type=int, default=0)
    ap.add_argument("--hq-quality", type=int, default=60)
    ap.add_argument("--hq-fps", type=int, default=10)
    ap.add_argument("--mode-file", default="", help="file the rover keeps updated with 'home', 'hotspot' or 'none'")
    ap.add_argument("--threshold", type=float, default=0.4, help="minimum confidence (0-1)")
    ap.add_argument("--max-det", type=int, default=15)
    ap.add_argument("--hold", type=float, default=0.7, help="keep a box this many seconds after it was last seen (stops flicker)")
    ap.add_argument("--smooth", type=float, default=0.5, help="box smoothing 0-1 (higher = follows the detection faster, lower = steadier)")
    ap.add_argument("--keep-ratio", type=float, default=1.0, help="an already-tracked object stays alive down to threshold*this (1.0 = threshold is a hard floor)")
    ap.add_argument("--rotate", type=int, default=180, choices=(0, 180), help="rover's camera is mounted upside down")
    ap.add_argument("--labels", default="", help="optional text file, one class name per line (default: 80 COCO names)")
    ap.add_argument("--record-dir", default=os.environ.get("ROVER_RECORD_DIR", ""),
                    help="save everything sent to the browser here (default: env ROVER_RECORD_DIR; empty = off)")
    ap.add_argument("--record-max-mb", type=int, default=int(os.environ.get("ROVER_RECORD_MAX_MB", "6000")),
                    help="stop recording this boot after this many MB (protects the SD card)")
    ap.add_argument("--record-keep", type=int, default=3, help="how many earlier boots to keep")
    ap.add_argument("--debug", action="store_true", help="print the model's top raw scores once a second to stderr (testing)")
    ap.add_argument("--max-frames", type=int, default=0, help="stop after N frames (testing)")
    args = ap.parse_args()

    import numpy as np
    from PIL import Image, ImageDraw, ImageFont
    from picamera2 import Picamera2
    from picamera2.devices.imx500 import IMX500

    names = COCO80
    if args.labels:
        names = [l.strip() for l in open(args.labels) if l.strip()]
    names = [n.replace("_", " ") for n in names]

    log("loading model onto the camera (this takes a while)...")
    imx500 = IMX500(args.model)
    in_w, in_h = imx500.get_input_size()
    picam2 = Picamera2(imx500.camera_num)
    hq_on = args.hq_width > 0 and args.hq_height > 0 and bool(args.mode_file)
    cap_w, cap_h = (args.hq_width, args.hq_height) if hq_on else (args.width, args.height)
    cam_fps = max(args.fps, args.hq_fps) if hq_on else args.fps
    if hq_on and abs(args.width * args.hq_height - args.height * args.hq_width) > 0.02 * args.width * args.hq_height:
        log("WARNING: the normal size %dx%d and the high-quality size %dx%d have different shapes; "
            "the picture will look stretched in one of them" % (args.width, args.height, args.hq_width, args.hq_height))
    cfg = picam2.create_video_configuration(main={"size": (cap_w, cap_h), "format": "BGR888"},
                                            controls={"FrameRate": cam_fps}, buffer_count=12)
    picam2.start(cfg)
    log("camera started, model input %dx%d, capturing %dx%d%s" % (
        in_w, in_h, cap_w, cap_h, " (high-quality profile available at home)" if hq_on else ""))
    fonts = {}

    def get_font(px):
        if px not in fonts:
            try:
                fonts[px] = ImageFont.load_default(size=px)
            except TypeError:
                fonts[px] = ImageFont.load_default()
        return fonts[px]

    mode, mode_t, last_hq, last_out = "none", 0.0, None, 0.0

    rec, rec_path, rec_bytes, rec_sync = None, "", 0, 0.0
    if args.record_dir:
        try:
            rec, rec_path = open_recorder(args.record_dir, args.record_keep)
            rec_bytes = os.path.getsize(rec_path)
            log("recording to", rec_path, "(stops at %d MB)" % args.record_max_mb)
        except OSError as e:
            log("cannot record (video continues):", repr(e))
    tracks, warned, nframes, last_dbg, nout = [], False, 0, 0.0, 0
    low = args.threshold * args.keep_ratio

    def iou(a, b):
        ax1, ay1, bx1, by1 = a[0] + a[2], a[1] + a[3], b[0] + b[2], b[1] + b[3]
        iw = min(ax1, bx1) - max(a[0], b[0]); ih = min(ay1, by1) - max(a[1], b[1])
        if iw <= 0 or ih <= 0:
            return 0.0
        inter = iw * ih
        return inter / float(a[2] * a[3] + b[2] * b[3] - inter + 1e-9)

    out = sys.stdout.buffer
    while True:
        req = picam2.capture_request()
        try:
            md = req.get_metadata()
            frame = req.make_array("main").copy()
        finally:
            req.release()
        H, W = frame.shape[0], frame.shape[1]

        try:
            outs = imx500.get_outputs(md, add_batch=False)
            if outs is not None and len(outs) >= 3:
                boxes = np.asarray(outs[0], dtype=np.float32).reshape(-1, 4)
                scores = np.asarray(outs[1], dtype=np.float32).reshape(-1)
                classes = np.asarray(outs[2]).reshape(-1)
                n = min(len(boxes), len(scores), len(classes))
                now = time.time()
                nout += 1
                if args.debug and not getattr(main, "_dumped", False):
                    main._dumped = True
                    for k, o in enumerate(outs):
                        a = np.asarray(o)
                        log("output[%d] shape %s dtype %s min %.4g max %.4g first: %s" % (
                            k, a.shape, a.dtype, float(a.min()) if a.size else 0, float(a.max()) if a.size else 0,
                            np.array2string(a.reshape(-1)[:6], precision=5)))
                if args.debug and now - last_dbg >= 1.0:
                    last_dbg = now
                    top = np.argsort(-scores[:n])[:5]
                    log("model results/s ~%d, top raw: %s" % (nout, ", ".join(
                        "%s %.3e" % (names[int(classes[i])] if 0 <= int(classes[i]) < len(names) else int(classes[i]), scores[i])
                        for i in top)))
                    nout = 0
                cands = []
                for i in np.argsort(-scores[:n])[:args.max_det * 3]:
                    if scores[i] < low:
                        break
                    x0, y0, x1, y1 = boxes[i]
                    nb = np.clip([y0 / in_h, x0 / in_w, y1 / in_h, x1 / in_w], 0.0, 1.0)
                    x, y, w, h = imx500.convert_inference_coords(tuple(float(v) for v in nb), md, picam2)
                    if w > 1 and h > 1:
                        cands.append([float(x), float(y), float(w), float(h), int(classes[i]), float(scores[i])])
                used = set()
                for t in tracks:
                    best, bi = 0.25, -1
                    for j, c in enumerate(cands):
                        if j in used or c[4] != t[4]:
                            continue
                        v = iou(t, c)
                        if v > best:
                            best, bi = v, j
                    if bi >= 0:
                        used.add(bi)
                        c = cands[bi]
                        a = args.smooth
                        for k in range(4):
                            t[k] = t[k] + a * (c[k] - t[k])
                        t[5] = c[5]
                        t[6] = now
                for j, c in enumerate(cands):
                    if j not in used and c[5] >= args.threshold:
                        tracks.append(c + [now])
        except Exception as e:
            if not warned:
                log("detection error (video continues without boxes):", repr(e))
                warned = True
        tnow = time.time()
        tracks = [t for t in tracks if tnow - t[6] <= args.hold]
        tracks.sort(key=lambda t: -t[5])
        dets = [(int(t[0]), int(t[1]), int(t[2]), int(t[3]), t[4], t[5]) for t in tracks[:args.max_det]]

        if hq_on and tnow - mode_t > 1.0:
            mode_t = tnow
            try:
                mode = open(args.mode_file).read().strip()
            except OSError:
                mode = "none"
        use_hq = hq_on and mode == "home"
        if use_hq != last_hq:
            if last_hq is not None or use_hq:
                log("picture profile: %s" % ("HIGH quality %dx%d q%d %dfps (home network)" % (
                    args.hq_width, args.hq_height, args.hq_quality, args.hq_fps) if use_hq else
                    "normal %dx%d q%d %dfps (%s)" % (args.width, args.height, args.quality, args.fps, mode)))
            last_hq = use_hq
        ow, oh, oq, ofps = ((args.hq_width, args.hq_height, args.hq_quality, args.hq_fps) if use_hq
                            else (args.width, args.height, args.quality, args.fps))
        if tnow - last_out < 1.0 / max(1, ofps) - 0.004:
            continue
        last_out = tnow
        sx, sy = ow / float(W), oh / float(H)
        font = get_font(max(14, ow // 34))
        lw = max(2, ow // 320)

        img = Image.fromarray(frame)
        if (ow, oh) != (W, H):
            img = img.resize((ow, oh), Image.BILINEAR)
        if args.rotate == 180:
            img = img.transpose(Image.ROTATE_180)
        dr = ImageDraw.Draw(img)
        for (x, y, w, h, c, s) in dets:
            if args.rotate == 180:
                x, y = W - x - w, H - y - h
            x, y, w, h = x * sx, y * sy, w * sx, h * sy
            col = COLORS[c % len(COLORS)]
            dr.rectangle([x, y, x + w, y + h], outline=col, width=lw)
            label = "%s %d%%" % (names[c] if 0 <= c < len(names) else "class%d" % c, round(s * 100))
            tw = dr.textlength(label, font=font)
            fh = font.size + 4 if hasattr(font, "size") else 18
            ty = y - fh if y >= fh else y
            dr.rectangle([x, ty, x + tw + 6, ty + fh - 1], fill=col)
            dr.text((x + 3, ty + 1), label, fill=(0, 0, 0), font=font)

        if rec:
            label = uptime_text()
            fh = font.size + 2 if hasattr(font, "size") else 16
            dr.rectangle([0, oh - fh, dr.textlength(label, font=font) + 8, oh], fill=(0, 0, 0))
            dr.text((4, oh - fh), label, fill=(255, 255, 255), font=font)
        buf = io.BytesIO()
        img.save(buf, "JPEG", quality=oq)
        data = buf.getvalue()
        try:
            out.write(data)
            out.flush()
        except BrokenPipeError:
            break
        if rec:
            try:
                if rec_bytes + len(data) > args.record_max_mb * 1024 * 1024:
                    log("recording size limit reached, recording stopped (video continues)")
                    rec.close(); rec = None
                else:
                    rec.write(data); rec_bytes += len(data)
                    rec.flush()
                    if time.time() - rec_sync > 2.0:
                        os.fsync(rec.fileno()); rec_sync = time.time()
            except OSError as e:
                log("recording stopped (video continues):", repr(e))
                rec = None
        nframes += 1
        if args.max_frames and nframes >= args.max_frames:
            break
    if rec:
        rec.close()
    picam2.stop()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
