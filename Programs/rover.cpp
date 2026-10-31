#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ctime>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <linux/gpio.h>
#include <linux/joystick.h>
#include <net/if.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "SCServo.h"
#include "httplib.h"

using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

namespace cfg {
constexpr double kPi = 3.14159265358979323846;

constexpr double HALF_WHEELBASE = 0.2;
constexpr double HALF_TRACK     = 0.1575;
constexpr double MAX_STEER_DEG  = 60.0;

constexpr int WHEEL_MAX_SPEED = 2000;
constexpr int WHEEL_ACC       = 75;

constexpr int    STEER_CENTER = 2048;
constexpr int    STEER_SPEED  = 2000;
constexpr int    STEER_ACC    = 40;
constexpr double STEER_RATE   = 0.8;
constexpr double ANALOG_SLEW  = 6.0;

constexpr int CAM_TILT_ID = 12, CAM_PAN_ID = 11;
constexpr int CAM_HOME = 2048;
constexpr int CAM_TILT_MIN = 1600, CAM_TILT_MAX = 2600;
constexpr int CAM_PAN_MIN  = 1024, CAM_PAN_MAX  = 3072;
constexpr int CAM_TILT_DIR = +1, CAM_PAN_DIR = +1;
constexpr double CAM_RATE  = 700.0;
constexpr int CAM_SPEED = 1500, CAM_ACC = 30;

constexpr double LOOP_HZ = 50.0;
constexpr auto REFRESH     = 200ms;
constexpr auto WEB_TIMEOUT = 600ms;

constexpr double DEADZONE = 0.12;
constexpr double DPAD_THROTTLE = 0.6;

constexpr int CAM_W = 480, CAM_H = 360, CAM_FPS = 15, CAM_QUALITY = 35;

constexpr const char* DETECT_JSON = "/usr/share/rpi-camera-assets/imx500_mobilenet_ssd.json";

constexpr double WHEEL_DIAMETER = 0.125;
constexpr double RV_HALF_LEN = 0.26;
constexpr double RV_HALF_WID = 0.2;
constexpr double HARD_MARGIN = 0.03;

constexpr double FRONT_X = 0.15, FRONT_Y = -0.085;
constexpr double SIDE_X  = 0.0575, SIDE_Y  = 0.1;
constexpr double BACK_X  = -0.075, BACK_Y = 0.0925;

constexpr int FRONT_TRIG = 5,  FRONT_ECHO = 6;
constexpr int SIDE_TRIG  = 17, SIDE_ECHO  = 27;
constexpr int BACK_TRIG  = 23, BACK_ECHO  = 24;
constexpr int GPIOCHIP = -1;

constexpr double SONAR_MAX = 4.5, SONAR_MIN = 0.02;
constexpr auto SONAR_TRIGGER_HIGH = 12us;

constexpr auto SONAR_RISE_WAIT = 60ms;
constexpr auto SONAR_FALL_WAIT = 50ms;

constexpr bool SONAR_ECHO_PULLUP = true;
constexpr auto SONAR_INTER_PING = 25ms;
constexpr double SONAR_FAULT_FRONT = 5.0, SONAR_FAULT_OTHER = 7.0;
constexpr double SONAR_CRAWL_AFTER = 1.5;
constexpr double SONAR_SPREAD = 0.05;
constexpr double CARVE_CONE = 0.10;
constexpr double CORE_CONE = 0.15;
constexpr double BEAM_HALF = 0.26;
constexpr double SQ_CREDIT = 0.10;
constexpr double SQ_MIN = 0.40;
constexpr double NOECHO_CONE = 0.12;
constexpr double PT_LIFE = 8.0;
constexpr double L_LOOK = 1.2, L_REV = 0.8;
constexpr double S_MIN = 0.12;
constexpr double REV_MIN = 0.25, REV_DIST = 0.4;
constexpr double REV_EXTRA_MARGIN = 0.03;
constexpr double K_SLEW = 3.0;
constexpr double AUTO_THR_MAX = 0.5, AUTO_THR_MIN = 0.12;
constexpr double CAL_THR = 0.35, REV_THR = 0.3;
constexpr double SWEEP_AMP = 45.0, SWEEP_PERIOD = 5.0;
constexpr double PAN_MAX_DEG = 70.0;
constexpr int AUTO_PAN_SPEED = 3000, AUTO_PAN_ACC = 100;
constexpr double ARRIVE_RADIUS = 2.5;
constexpr double HDOP_MAX = 4.0;
constexpr double HDG_BASELINE = 8.0;
constexpr double LINK_TIMEOUT = 10.0;
}

struct Wheel { uint8_t id; double x, y; int dir; };

static constexpr double WB = cfg::HALF_WHEELBASE, TW = cfg::HALF_TRACK;

static const Wheel DRIVE[6] = {
    {1, +WB, -TW, +1}, {2, +WB, +TW, -1},
    {3, 0.0, -TW, +1}, {4, 0.0, +TW, -1},
    {5, -WB, -TW, +1}, {6, -WB, +TW, -1}};

static const Wheel STEER[4] = {
    {7,  +WB, -TW, +1}, {8,  +WB, +TW, +1},
    {9,  -WB, -TW, +1}, {10, -WB, +TW, +1}};
static const int STEER_TRIM[4] = {0, 0, 0, 0};

static int g_zero[6] = {cfg::STEER_CENTER, cfg::STEER_CENTER, cfg::STEER_CENTER,
                        cfg::STEER_CENTER, cfg::CAM_HOME, cfg::CAM_HOME};

static const double K_MAX = [] {
    const double t = std::tan(cfg::MAX_STEER_DEG * cfg::kPi / 180.0);
    return t / (WB + TW * t);
}();

static std::atomic<bool> g_run{true};
static void onSignal(int) { g_run = false; }
static void napMs(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
static double clampd(double v, double lo, double hi) { return v < lo ? lo : (v > hi ? hi : v); }

struct Input {
    double thr = 0;
    double str_analog = 0;
    int    str_dir = 0;
    double pan = 0, tilt = 0;
    bool   ctr_steer = false, ctr_cam = false;
    Clock::time_point stamp{};
};
static struct { std::mutex m; Input pad, web; } g;

struct AutoCmd { bool active = false; double thr = 0, s = 0, pan_deg = 0; };
static struct { std::mutex m; AutoCmd cmd; } g_auto;
static std::atomic<int> g_netMode{0};
static std::atomic<double> g_lastClient{0.0};
static std::mutex g_netM;
static std::string g_password;
static bool g_multi = false;
static std::atomic<bool> g_estop{false};
static std::atomic<bool> g_autoCancel{false};
static std::atomic<double> g_vbody{NAN};
static std::atomic<double> g_ratio{-1.0};
static std::atomic<double> g_panDeg{0.0};
static std::atomic<double> g_kcmd{0.0};
static std::mutex g_statusM;
static std::string g_status = "{}";
static bool g_camAlways = false;
static double steadyS() { return std::chrono::duration<double>(Clock::now().time_since_epoch()).count(); }
static std::atomic<double> g_ctlLagUntil{0.0};
static std::atomic<double> g_lastCtlT{0.0};
static std::atomic<int>    g_lastCtlK{0};

struct FrameStore {
    std::mutex m;
    std::condition_variable cv;
    std::shared_ptr<const std::string> jpg;
    uint64_t seq = 0;
};
static FrameStore g_frame;

static std::mutex g_vm;
static std::condition_variable g_vcv;
static int g_viewers = 0;
struct Viewer {
    Viewer()  { { std::lock_guard<std::mutex> l(g_vm); ++g_viewers; } g_vcv.notify_all(); }
    ~Viewer() { { std::lock_guard<std::mutex> l(g_vm); --g_viewers; } g_vcv.notify_all(); }
};
static int viewers() { std::lock_guard<std::mutex> l(g_vm); return g_viewers; }

struct Pkt { int16_t speed[6]; int16_t pos[6]; };

static void solve(double s, double v, Pkt& out) {
    const double k = -s * K_MAX;
    double fac[6], fmax = 1.0;
    for (int i = 0; i < 6; i++) {
        fac[i] = std::hypot(DRIVE[i].x * k, 1.0 - DRIVE[i].y * k);
        fmax = std::max(fmax, fac[i]);
    }
    for (int i = 0; i < 6; i++)
        out.speed[i] = (int16_t)std::lround(DRIVE[i].dir * v * cfg::WHEEL_MAX_SPEED * fac[i] / fmax);

    const double stepsPerRad = 4096.0 / (2.0 * cfg::kPi);
    for (int j = 0; j < 4; j++) {
        const double d = std::atan2(STEER[j].x * k, 1.0 - STEER[j].y * k);
        const long p = std::lround(g_zero[j] + STEER_TRIM[j] + STEER[j].dir * d * stepsPerRad);
        out.pos[j] = (int16_t)std::min(4095L, std::max(0L, p));
    }
}

template <class B>
static auto syncWriteCompat(B& b, uint8_t* ids, uint8_t n, uint8_t addr, uint8_t* d, uint8_t len, int)
    -> decltype(b.syncWrite(ids, n, addr, d, len), void()) { b.syncWrite(ids, n, addr, d, len); }
template <class B>
static void syncWriteCompat(B& b, uint8_t* ids, uint8_t n, uint8_t addr, uint8_t* d, uint8_t len, long) {
    b.snycWrite(ids, n, addr, d, len);
}

static void syncSpeeds(SMS_STS& bus, const int16_t sp[6], uint8_t acc = cfg::WHEEL_ACC) {
    uint8_t ids[6] = {1, 2, 3, 4, 5, 6};
    uint8_t buf[6 * 7];
    for (int i = 0; i < 6; i++) {
        uint8_t* b = buf + i * 7;
        const int mag = std::min(std::abs((int)sp[i]), 0x7fff);
        const uint16_t u = (uint16_t)(sp[i] < 0 ? (mag | 0x8000) : mag);
        b[0] = acc;
        b[1] = b[2] = b[3] = b[4] = 0;
        b[5] = (uint8_t)(u & 0xff);
        b[6] = (uint8_t)(u >> 8);
    }
    syncWriteCompat(bus, ids, 6, 41, buf, 7, 0);
}

static bool setMode(SMS_STS& bus, uint8_t id, uint8_t want) {
    for (int attempt = 0; attempt < 3; attempt++) {
        const int m = bus.readByte(id, 33);
        if (m < 0) { napMs(30); continue; }
        if (m == want) return true;
        bus.writeByte(id, 55, 0);
        napMs(5);
        bus.writeByte(id, 33, want);
        napMs(5);
        bus.writeByte(id, 55, 1);
        napMs(10);
    }
    return bus.readByte(id, 33) == want;
}

static void initServos(SMS_STS& bus, double& tilt0, double& pan0) {
    std::string badWheel, badPos;
    for (int id = 1; id <= 6; id++) {
        if (setMode(bus, (uint8_t)id, 1)) {
            bus.WriteSpe((uint8_t)id, 0, 0);
            bus.EnableTorque((uint8_t)id, 1);
        } else badWheel += " " + std::to_string(id);
    }
    for (int id = 7; id <= 12; id++) {
        if (setMode(bus, (uint8_t)id, 0)) {
            const int pos = bus.ReadPos(id);
            if (pos >= 0) {
                const int p = std::min(pos, 4095);
                bus.WritePosEx((uint8_t)id, (int16_t)p, 1000, 20);
                g_zero[id <= 10 ? id - 7 : (id == cfg::CAM_TILT_ID ? 4 : 5)] = p;
                if (id == cfg::CAM_TILT_ID) tilt0 = p;
                if (id == cfg::CAM_PAN_ID)  pan0 = p;
            }
            bus.EnableTorque((uint8_t)id, 1);
        } else badPos += " " + std::to_string(id);
    }
    if (badWheel.empty()) std::fprintf(stderr, "[init] IDs 1-6 : wheel mode OK\n");
    else std::fprintf(stderr, "[init] WARNING: no reply / mode not confirmed for wheel IDs:%s\n", badWheel.c_str());
    if (badPos.empty()) std::fprintf(stderr, "[init] IDs 7-12: position mode OK\n");
    else std::fprintf(stderr, "[init] WARNING: no reply / mode not confirmed for position IDs:%s\n", badPos.c_str());
    std::fprintf(stderr, "[init] zero saved (steer 7-10, tilt, pan): %d %d %d %d %d %d  - restored on exit\n",
                 g_zero[0], g_zero[1], g_zero[2], g_zero[3], g_zero[4], g_zero[5]);
}

static void controlLoop(SMS_STS& bus, double tilt0, double pan0) {
    uint8_t  pid[6] = {7, 8, 9, 10, cfg::CAM_TILT_ID, cfg::CAM_PAN_ID};
    uint8_t  acc[6];
    uint16_t spd[6];
    for (int i = 0; i < 6; i++) {
        acc[i] = i < 4 ? cfg::STEER_ACC : cfg::CAM_ACC;
        spd[i] = i < 4 ? cfg::STEER_SPEED : cfg::CAM_SPEED;
    }

    const double dt = 1.0 / cfg::LOOP_HZ;
    const auto period = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(dt));
    double s = 0;
    const double tiltLo = clampd(g_zero[4] + cfg::CAM_TILT_MIN - cfg::CAM_HOME, 0, 4095);
    const double tiltHi = clampd(g_zero[4] + cfg::CAM_TILT_MAX - cfg::CAM_HOME, 0, 4095);
    const double panLo  = clampd(g_zero[5] + cfg::CAM_PAN_MIN  - cfg::CAM_HOME, 0, 4095);
    const double panHi  = clampd(g_zero[5] + cfg::CAM_PAN_MAX  - cfg::CAM_HOME, 0, 4095);
    double tilt = clampd(tilt0, tiltLo, tiltHi);
    double pan  = clampd(pan0,  panLo,  panHi);
    bool analogActive = false, first = true, prevAuto = false, prevEstop = false;
    unsigned tick = 0;
    Pkt cur{}, last{};
    auto lastSend = Clock::now();
    auto next = Clock::now();

    while (g_run) {
        next += period;
        std::this_thread::sleep_until(next);
        const auto now = Clock::now();

        Input pad, web;
        {
            std::lock_guard<std::mutex> lk(g.m);
            pad = g.pad;  web = g.web;
            g.pad.ctr_steer = g.pad.ctr_cam = false;
            g.web.ctr_steer = g.web.ctr_cam = false;
        }
        if (now - web.stamp > cfg::WEB_TIMEOUT) web = Input{};

        AutoCmd ac;
        { std::lock_guard<std::mutex> lk(g_auto.m); ac = g_auto.cmd; }
        const bool estop  = g_estop.load();
        const bool autoOn = ac.active && !estop;
        if (autoOn && (std::fabs(pad.thr) > 0.3 || std::fabs(pad.str_analog) > 0.3 || pad.str_dir != 0))
            g_autoCancel = true;

        double thr = clampd(pad.thr + web.thr, -1, 1);
        const int    dir = (int)clampd(pad.str_dir + web.str_dir, -1, 1);
        const double ana = clampd(pad.str_analog + web.str_analog, -1, 1);
        const double cpan  = clampd(pad.pan + web.pan, -1, 1);
        const double ctilt = clampd(pad.tilt + web.tilt, -1, 1);

        const double slew = cfg::ANALOG_SLEW * dt;
        if (dir != 0) {
            s += dir * cfg::STEER_RATE * dt;
            analogActive = false;
        } else if (std::fabs(ana) > 0.02) {
            analogActive = true;
            s += clampd(ana - s, -slew, slew);
        } else if (analogActive) {
            s += clampd(0.0 - s, -slew, slew);
            if (std::fabs(s) < 1e-3) { s = 0; analogActive = false; }
        }
        if (pad.ctr_steer || web.ctr_steer) { s = 0; analogActive = false; }
        s = clampd(s, -1, 1);

        tilt = clampd(tilt + cfg::CAM_TILT_DIR * ctilt * cfg::CAM_RATE * dt, tiltLo, tiltHi);
        pan  = clampd(pan  + cfg::CAM_PAN_DIR  * cpan  * cfg::CAM_RATE * dt, panLo,  panHi);
        if (pad.ctr_cam || web.ctr_cam) { tilt = g_zero[4]; pan = g_zero[5]; }

        if (autoOn) {
            thr = clampd(ac.thr, -1, 1);
            s = clampd(ac.s, -1, 1);
            analogActive = false;
            tilt = g_zero[4];
            pan = clampd(g_zero[5] - ac.pan_deg * cfg::CAM_PAN_DIR * 4096.0 / 360.0, panLo, panHi);
            spd[5] = cfg::AUTO_PAN_SPEED; acc[5] = cfg::AUTO_PAN_ACC;
        } else {
            if (prevAuto) { s = 0; tilt = g_zero[4]; pan = g_zero[5]; }
            spd[5] = cfg::CAM_SPEED; acc[5] = cfg::CAM_ACC;
        }
        prevAuto = autoOn;
        if (estop) thr = 0;

        solve(s, thr, cur);
        cur.pos[4] = (int16_t)std::lround(tilt);
        cur.pos[5] = (int16_t)std::lround(pan);
        g_kcmd = -s * K_MAX;

        const bool refresh = now - lastSend > cfg::REFRESH;
        if (estop && !prevEstop) {
            syncSpeeds(bus, cur.speed, 0);
            syncSpeeds(bus, cur.speed, 0);
        } else if (first || refresh || std::memcmp(cur.speed, last.speed, sizeof cur.speed) != 0)
            syncSpeeds(bus, cur.speed, estop ? 0 : cfg::WHEEL_ACC);
        prevEstop = estop;
        if (first || refresh || std::memcmp(cur.pos, last.pos, sizeof cur.pos) != 0)
            bus.SyncWritePosEx(pid, 6, cur.pos, spd, acc);
        if (first || refresh || std::memcmp(&cur, &last, sizeof cur) != 0) lastSend = now;
        last = cur;
        first = false;

        if (++tick % 5 == 0) {
            const int pp = bus.ReadPos(cfg::CAM_PAN_ID);
            if (pp >= 0) g_panDeg = -(pp - g_zero[5]) * cfg::CAM_PAN_DIR * 360.0 / 4096.0;
            const double k = -s * K_MAX;
            double vs = 0, rs = 0;
            int nv = 0, nr = 0;
            for (int i = 2; i <= 3; i++) {
                const int m = bus.ReadSpeed(DRIVE[i].id);
                const double c = cur.speed[i];
                if (std::fabs(c) > 150) { rs += m / c; ++nr; }
                const double fac = std::fabs(1.0 - DRIVE[i].y * k);
                if (fac > 0.2) { vs += DRIVE[i].dir * (double)m / fac; ++nv; }
            }
            g_vbody = nv ? (vs / nv) * cfg::kPi * cfg::WHEEL_DIAMETER / 4096.0 : NAN;
            g_ratio = nr ? rs / nr : -1.0;
        }
    }

    const int16_t zero[6] = {0, 0, 0, 0, 0, 0};
    syncSpeeds(bus, zero);
    napMs(20);
    syncSpeeds(bus, zero);

    int16_t home[6];
    for (int i = 0; i < 6; i++) home[i] = (int16_t)g_zero[i];
    bool arrived = false;
    for (int t = 0; t < 30 && !arrived; t++) {
        if (t % 5 == 0) bus.SyncWritePosEx(pid, 6, home, spd, acc);
        napMs(100);
        arrived = true;
        for (int i = 0; i < 6; i++) {
            const int p = bus.ReadPos(pid[i]);
            if (p >= 0 && std::abs(p - g_zero[i]) > 15) arrived = false;
        }
    }
    std::fprintf(stderr, arrived ? "[exit] steering and camera returned to zero\n"
                                 : "[exit] WARNING: some servos did not reach zero\n");
}

struct PadMap {
    int lx = -1, ly = -1, rx = -1, ry = -1, hx = -1, hy = -1;
    int bB = -1, bLB = -1, bRB = -1, bR3 = -1, bU = -1, bD = -1, bL = -1, bR = -1;
};

static PadMap readPadMap(int fd, char* name, size_t nlen) {
    PadMap m;
    std::snprintf(name, nlen, "unknown");
    ioctl(fd, JSIOCGNAME((int)nlen), name);

    uint8_t nax = 0, nbt = 0;
    ioctl(fd, JSIOCGAXES, &nax);
    ioctl(fd, JSIOCGBUTTONS, &nbt);
    uint8_t  axmap[ABS_CNT];
    uint16_t btmap[KEY_MAX - BTN_MISC + 1];
    if (ioctl(fd, JSIOCGAXMAP, axmap) < 0 || ioctl(fd, JSIOCGBTNMAP, btmap) < 0) {
        m.lx = 0; m.ly = 1; m.rx = 3; m.ry = 4; m.hx = 6; m.hy = 7;
        m.bB = 1; m.bLB = 4; m.bRB = 5; m.bR3 = 10;
        return m;
    }

    int idx[ABS_CNT];
    std::fill(idx, idx + ABS_CNT, -1);
    for (int i = 0; i < nax && i < ABS_CNT; i++)
        if (axmap[i] < ABS_CNT) idx[axmap[i]] = i;
    m.lx = idx[ABS_X];  m.ly = idx[ABS_Y];
    if (idx[ABS_RX] >= 0 && idx[ABS_RY] >= 0) { m.rx = idx[ABS_RX]; m.ry = idx[ABS_RY]; }
    else                                      { m.rx = idx[ABS_Z];  m.ry = idx[ABS_RZ]; }
    m.hx = idx[ABS_HAT0X];  m.hy = idx[ABS_HAT0Y];

    for (int j = 0; j < nbt; j++) {
        switch (btmap[j]) {
            case BTN_EAST:      m.bB  = j; break;
            case BTN_TL:        m.bLB = j; break;
            case BTN_TR:        m.bRB = j; break;
            case BTN_THUMBR:    m.bR3 = j; break;
            case BTN_DPAD_UP:   m.bU  = j; break;
            case BTN_DPAD_DOWN: m.bD  = j; break;
            case BTN_DPAD_LEFT: m.bL  = j; break;
            case BTN_DPAD_RIGHT:m.bR  = j; break;
            default: break;
        }
    }
    return m;
}

static void padThread(std::string dev) {
    static const double GEAR[4] = {0.25, 0.5, 0.75, 1.0};
    int gear = 1;
    auto dz = [](double v) {
        const double a = std::fabs(v);
        return a < cfg::DEADZONE ? 0.0 : std::copysign((a - cfg::DEADZONE) / (1.0 - cfg::DEADZONE), v);
    };

    while (g_run) {
        const int fd = open(dev.c_str(), O_RDONLY);
        if (fd < 0) { napMs(1000); continue; }
        char name[128];
        const PadMap m = readPadMap(fd, name, sizeof name);
        std::fprintf(stderr, "[pad] connected: %s (%s) L=%d,%d R=%d,%d hat=%d,%d\n",
                     dev.c_str(), name, m.lx, m.ly, m.rx, m.ry, m.hx, m.hy);

        double ax[ABS_CNT] = {0};
        bool   bt[256] = {false};
        auto A = [&](int i) { return i >= 0 ? ax[i] : 0.0; };
        auto B = [&](int i) { return (i >= 0 && bt[i]) ? 1.0 : 0.0; };
        pollfd pf{fd, POLLIN, 0};

        while (g_run) {
            const int r = poll(&pf, 1, 200);
            if (r == 0) continue;
            if (r < 0) { if (errno == EINTR) continue; break; }
            js_event e;
            if (read(fd, &e, sizeof e) != (ssize_t)sizeof e) break;

            const uint8_t type = e.type & ~JS_EVENT_INIT;
            const bool live = !(e.type & JS_EVENT_INIT);
            bool evSteer = false, evCam = false;
            if (type == JS_EVENT_AXIS) {
                if (e.number < ABS_CNT) ax[e.number] = e.value / 32767.0;
            } else if (type == JS_EVENT_BUTTON) {
                bt[e.number] = e.value != 0;
                if (e.value && live) {
                    if (e.number == m.bB)  evSteer = true;
                    if (e.number == m.bR3) evCam = true;
                    if (e.number == m.bLB) gear = std::max(0, gear - 1);
                    if (e.number == m.bRB) gear = std::min(3, gear + 1);
                }
            }

            const double hatx = m.hx >= 0 ? A(m.hx) : B(m.bR) - B(m.bL);
            const double haty = m.hy >= 0 ? A(m.hy) : B(m.bD) - B(m.bU);
            const double ly = -dz(A(m.ly));
            const double thr = (ly != 0.0 ? ly : -haty * cfg::DPAD_THROTTLE) * GEAR[gear];

            std::lock_guard<std::mutex> lk(g.m);
            Input& p = g.pad;
            p.thr = thr;
            p.str_analog = dz(A(m.lx));
            p.str_dir = (int)std::lround(hatx);
            p.pan  = dz(A(m.rx));
            p.tilt = -dz(A(m.ry));
            if (evSteer) p.ctr_steer = true;
            if (evCam)   p.ctr_cam = true;
        }
        close(fd);
        std::fprintf(stderr, "[pad] disconnected\n");
        std::lock_guard<std::mutex> lk(g.m);
        g.pad = Input{};
    }
}

static const char* const kModeFile = "/tmp/rover-netmode";
static bool writeModeFile(int mode, bool lagging = false) {
    const char* word = mode == 1 ? (lagging ? "lag" : "home") : mode == 2 ? "hotspot" : "none";
    const std::string tmp = std::string(kModeFile) + ".tmp";
    FILE* f = std::fopen(tmp.c_str(), "w");
    if (!f) return false;
    std::fputs(word, f);
    std::fclose(f);
    return std::rename(tmp.c_str(), kModeFile) == 0;
}
static void modeFileThread() {
    int last = -1;
    bool lastLag = false;
    while (g_run) {
        const int m = g_netMode.load();
        const bool lag = steadyS() < g_ctlLagUntil.load();
        if ((m != last || lag != lastLag) && writeModeFile(m, lag)) { last = m; lastLag = lag; }
        for (int i = 0; i < 5 && g_run; i++) napMs(100);
    }
}

static void publish(std::string&& jpg) {
    auto sp = std::make_shared<const std::string>(std::move(jpg));
    {
        std::lock_guard<std::mutex> lk(g_frame.m);
        g_frame.jpg = std::move(sp);
        ++g_frame.seq;
    }
    g_frame.cv.notify_all();
}

struct Child { pid_t pid = -1; int fd = -1; };

static Child spawn(const std::string& cmd) {
    int pfd[2];
    if (pipe(pfd) < 0) return {};
    const pid_t pid = fork();
    if (pid < 0) { close(pfd[0]); close(pfd[1]); return {}; }
    if (pid == 0) {
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        dup2(pfd[1], 1);
        close(pfd[0]);
        close(pfd[1]);
        execl("/bin/sh", "sh", "-c", cmd.c_str(), (char*)nullptr);
        _exit(127);
    }
    close(pfd[1]);
    return {pid, pfd[0]};
}

static void reap(Child& c) {
    if (c.fd >= 0) close(c.fd);
    if (c.pid > 0) { kill(c.pid, SIGTERM); waitpid(c.pid, nullptr, 0); }
    c = Child{};
}

static void camThread(std::string cmd) {
    bool warned = false;
    static char chunk[1 << 16];
    while (g_run) {
        {
            std::unique_lock<std::mutex> lk(g_vm);
            g_vcv.wait(lk, [] { return g_viewers > 0 || g_camAlways || !g_run; });
        }
        if (!g_run) break;

        Child c = spawn(cmd);
        std::string buf;
        buf.reserve(1 << 19);
        uint64_t frames = 0;
        ssize_t n;
        while (c.fd >= 0 && g_run && (viewers() > 0 || g_camAlways) && (n = read(c.fd, chunk, sizeof chunk)) > 0) {
            buf.append(chunk, (size_t)n);
            size_t pos = 0;
            for (;;) {
                const size_t a = buf.find("\xff\xd8", pos, 2);
                if (a == std::string::npos) { pos = buf.empty() ? 0 : buf.size() - 1; break; }
                const size_t b = buf.find("\xff\xd9", a + 2, 2);
                if (b == std::string::npos) { pos = a; break; }
                publish(buf.substr(a, b + 2 - a));
                ++frames;
                pos = b + 2;
            }
            buf.erase(0, pos);
        }
        reap(c);
        {
            std::lock_guard<std::mutex> lk(g_frame.m);
            g_frame.jpg.reset();
        }
        if (g_run && frames == 0 && (viewers() > 0 || g_camAlways)) {
            if (!warned)
                std::fprintf(stderr, "[cam] no frames. Try the command by hand: %s\n", cmd.c_str());
            warned = true;
            for (int i = 0; i < 20 && g_run; i++) napMs(100);
        }
    }
}

struct Net { in_addr_t ip = 0, mask = 0; bool ok = false; };
static Net g_net;

static Net netFind(const std::string& iface, in_addr_t wantIp) {
    Net r;
    ifaddrs* all = nullptr;
    if (getifaddrs(&all) != 0) return r;
    for (ifaddrs* a = all; a; a = a->ifa_next) {
        if (!a->ifa_addr || a->ifa_addr->sa_family != AF_INET || !a->ifa_netmask) continue;
        const in_addr_t ip = ((sockaddr_in*)a->ifa_addr)->sin_addr.s_addr;
        const bool match = iface.empty() ? (ip == wantIp) : (iface == a->ifa_name);
        if (!match) continue;
        r.ip = ip;
        r.mask = ((sockaddr_in*)a->ifa_netmask)->sin_addr.s_addr;
        r.ok = true;
        break;
    }
    freeifaddrs(all);
    return r;
}

static std::string ipStr(in_addr_t ip) {
    char b[INET_ADDRSTRLEN] = "?";
    in_addr a; a.s_addr = ip;
    inet_ntop(AF_INET, &a, b, sizeof b);
    return b;
}

static int runCmd(const std::vector<std::string>& args, std::string* out = nullptr) {
    std::vector<char*> av;
    for (const auto& a : args) av.push_back(const_cast<char*>(a.c_str()));
    av.push_back(nullptr);
    int pfd[2] = {-1, -1};
    if (out && pipe(pfd) < 0) return -1;
    const pid_t pid = fork();
    if (pid < 0) { if (out) { close(pfd[0]); close(pfd[1]); } return -1; }
    if (pid == 0) {
        if (out) { dup2(pfd[1], 1); close(pfd[0]); close(pfd[1]); }
        execvp(av[0], av.data());
        _exit(127);
    }
    if (out) {
        close(pfd[1]);
        char b[256];
        ssize_t n;
        while ((n = read(pfd[0], b, sizeof b)) > 0) out->append(b, (size_t)n);
        close(pfd[0]);
    }
    int st = 0;
    waitpid(pid, &st, 0);
    return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

static void listIfaces() {
    ifaddrs* all = nullptr;
    if (getifaddrs(&all) != 0) return;
    for (ifaddrs* a = all; a; a = a->ifa_next)
        if (a->ifa_addr && a->ifa_addr->sa_family == AF_INET)
            std::fprintf(stderr, "[web]   %-10s %s\n", a->ifa_name,
                         ipStr(((sockaddr_in*)a->ifa_addr)->sin_addr.s_addr).c_str());
    freeifaddrs(all);
}

static Net resolveNet(const std::string& iface, const std::string& bindIp, const std::string& hotspot) {
    in_addr want{};
    if (!bindIp.empty() && inet_pton(AF_INET, bindIp.c_str(), &want) != 1) {
        std::fprintf(stderr, "[web] bad --bind address: %s\n", bindIp.c_str());
        return Net{};
    }
    const bool needHotspot = !hotspot.empty() && bindIp.empty();
    bool started = false, announced = false;
    for (int i = 0; i < 60 && g_run; i++) {
        if (needHotspot) {
            std::string cur;
            runCmd({"nmcli", "-g", "GENERAL.CONNECTION", "device", "show", iface}, &cur);
            while (!cur.empty() && (cur.back() == '\n' || cur.back() == '\r')) cur.pop_back();
            if (cur != hotspot) {
                if (!started) {
                    started = true;
                    std::fprintf(stderr, "[net] %s is not running hotspot '%s' - starting it...\n",
                                 iface.c_str(), hotspot.c_str());
                    const int rc = runCmd({"nmcli", "connection", "up", hotspot});
                    if (rc != 0)
                        std::fprintf(stderr, "[net] 'nmcli connection up %s' failed (exit %d): "
                                             "does that connection exist, and is nmcli allowed?\n",
                                     hotspot.c_str(), rc);
                }
                napMs(1000);
                continue;
            }
        }
        const Net n = bindIp.empty() ? netFind(iface, 0) : netFind("", want.s_addr);
        if (n.ok) return n;
        if (!announced) {
            announced = true;
            std::fprintf(stderr, "[web] waiting for %s to get an IPv4 address...\n",
                         bindIp.empty() ? iface.c_str() : bindIp.c_str());
        }
        napMs(1000);
    }
    return Net{};
}

static bool clientAllowed(const std::string& remote) {
    in_addr a;
    if (inet_pton(AF_INET, remote.c_str(), &a) != 1) return false;
    if ((ntohl(a.s_addr) >> 24) == 127) return true;
    std::lock_guard<std::mutex> lk(g_netM);
    return (a.s_addr & g_net.mask) == (g_net.ip & g_net.mask);
}

static bool onOwnHotspot(const std::string& remote) {
    in_addr a;
    if (inet_pton(AF_INET, remote.c_str(), &a) != 1) return false;
    if ((ntohl(a.s_addr) >> 24) == 127) return false;
    if (g_netMode.load() != 2) return false;
    std::lock_guard<std::mutex> lk(g_netM);
    return g_net.ok && (a.s_addr & g_net.mask) == (g_net.ip & g_net.mask);
}

static void printLinks(int port) {
    char host[256] = "raspberrypi";
    gethostname(host, sizeof host - 1);
    host[sizeof host - 1] = 0;
    const std::string ip = ipStr(g_net.ip);
    std::fprintf(stderr,
        "\n================================================================\n"
        " Rover web control is running (no internet needed)\n"
        "   Control + video : http://%s:%d/\n"
        "   Video only      : http://%s:%d/stream\n"
        "   By name (mDNS)  : http://%s.local:%d/\n"
        " Open one from a device joined to the Pi's own Wi-Fi network.\n"
        "================================================================\n\n",
        ip.c_str(), port, ip.c_str(), port, host, port);
}

namespace au {
using cfg::kPi;

static double wrapPi(double a) {
    while (a > kPi) a -= 2 * kPi;
    while (a < -kPi) a += 2 * kPi;
    return a;
}
static double rad(double d) { return d * kPi / 180.0; }
static double deg(double r) { return r * 180.0 / kPi; }

enum Src { FRONT = 0, SIDE = 1, BACK = 2 };
struct Mount { double x, y, ang; };
static const Mount MOUNT[3] = {
    {cfg::FRONT_X, cfg::FRONT_Y, 0.0},
    {cfg::SIDE_X, cfg::SIDE_Y, kPi / 2},
    {cfg::BACK_X, cfg::BACK_Y, kPi}};

struct Pt { double x, y, t; int src; double u = 0; };
struct Fix { bool ok = false; double lat = 0, lon = 0, hdop = 99, t = 0; };
struct Cmd { double thr = 0, s = 0, pan = 0; };
enum State { IDLE, CAL, NAV, RECOVER, ARRIVED, STUCK, FAULT };
static const char* stName(State s) {
    static const char* n[] = {"IDLE", "FINDING HEADING", "NAVIGATING", "BACKING UP", "ARRIVED", "STUCK", "FAULT"};
    return n[s];
}

constexpr int NK = 25;
constexpr int NS = 31;
struct Pose { double x, y, c, s; };

class Engine {
 public:
    State st = IDLE;
    std::string msg = "Idle";
    Fix fix;
    int gpsMode = 0;
    double gpsHdop = 99;
    bool hdgKnown = false;
    double hdg = 0;
    double dist = 0, brg = 0;
    double gx = 0, gy = 0;
    double rng[3] = {-1, -1, -1};
    double rngAng[3] = {0, 0, 0};
    double seen[3] = {-1e9, -1e9, -1e9};
    double speed = 0;
    std::vector<Pt> pts;

    Engine() {
        for (int i = 0; i < NK; i++) kTab[i] = -K_MAX + 2.0 * K_MAX * i / (NK - 1);
        ds = cfg::L_LOOK / (NS - 1);
        for (int i = 0; i < NK; i++)
            for (int j = 0; j < NS; j++) {
                fw[i][j] = pose(kTab[i], j * ds);
                bw[i][j] = pose(kTab[i], -j * ds);
            }
    }

    bool active() const { return st == CAL || st == NAV || st == RECOVER; }

    std::string start(double tlat, double tlon, bool headingGiven, double headingDeg, double now) {
        if (!fix.ok || now - fix.t > 3.0) return "No GPS fix yet - the VK-162 needs a clear view of the sky.";
        if (now - seen[FRONT] > 1.5 || now - seen[SIDE] > 2.5 || now - seen[BACK] > 2.5) {
            std::string w;
            if (now - seen[FRONT] > 1.5) w += " front";
            if (now - seen[SIDE] > 2.5) w += " side";
            if (now - seen[BACK] > 2.5) w += " back";
            return "Ultrasonic sensor(s) not responding:" + w + ". Each sensor must hear a real echo once after the "
                   "program starts: hold your hand ~30 cm in front of it, then press GO again.";
        }
        latO = fix.lat; lonO = fix.lon;
        posE = posN = 0; havePos = true;
        toENU(tlat, tlon, tE, tN);
        if (std::hypot(tE, tN) > 5000) return "Target is more than 5 km away.";
        hdgKnown = headingGiven;
        hdg = rad(headingDeg);
        pts.clear();
        anchorSet = false; reverseFlag = false; yawSince = 0; odomTotal = 0;
        stall = 0; lastThr = 0; kLast = 0; recoverTimes.clear();
        st = hdgKnown ? NAV : CAL;
        msg = hdgKnown ? "Heading to target" : "Driving straight to learn which way the rover faces";
        return "";
    }
    void cancel(const char* why) {
        st = IDLE; msg = why; kLast = 0; lastThr = 0;
    }

    void addRange(int src, bool hit, double r, double ang, double now) {
        seen[src] = now;
        rng[src] = hit ? r : 99.0;
        rngAng[src] = ang;
        const Mount& m = MOUNT[src];
        if (!hit) {
            for (size_t i = 0; i < pts.size();) {
                const Pt& q = pts[i];
                if (q.src == src) {
                    const double da = std::fabs(wrapPi(std::atan2(q.y - m.y, q.x - m.x) - ang));
                    if (da < cfg::NOECHO_CONE) { pts[i] = pts.back(); pts.pop_back(); continue; }
                }
                ++i;
            }
            return;
        }
        if (r < cfg::SONAR_MIN || r > cfg::SONAR_MAX) return;
        for (size_t i = 0; i < pts.size();) {
            const Pt& q = pts[i];
            if (q.src == src) {
                const double dx = q.x - m.x, dy = q.y - m.y;
                const double da = std::fabs(wrapPi(std::atan2(dy, dx) - ang)), rr = std::hypot(dx, dy);
                if ((da < cfg::CARVE_CONE && rr > r + 0.10) || (da < cfg::CORE_CONE && rr < r - 0.15)) {
                    pts[i] = pts.back(); pts.pop_back(); continue;
                }
            }
            ++i;
        }
        for (int d = -1; d <= 1; d++) {
            const double a = ang + d * cfg::SONAR_SPREAD;
            const Pt q{m.x + r * std::cos(a), m.y + r * std::sin(a), now, src, std::min(cfg::SQ_CREDIT, cfg::BEAM_HALF * r)};
            if (std::fabs(q.x) < cfg::RV_HALF_LEN + 0.02 && std::fabs(q.y) < cfg::RV_HALF_WID + 0.02)
                continue;
            pts.push_back(q);
        }
        if (pts.size() > 500) pts.erase(pts.begin(), pts.begin() + (long)(pts.size() - 500));
    }

    void noteGps(int mode, double hdop) { gpsMode = mode; gpsHdop = hdop; }

    void addGps(const Fix& f) {
        fix = f;
        if (!active()) return;
        double e, n;
        toENU(f.lat, f.lon, e, n);
        if (!havePos) { posE = e; posN = n; havePos = true; }
        else { posE += 0.5 * (e - posE); posN += 0.5 * (n - posN); }
        headingFromGps();
    }

    Cmd step(double now, double dt, double vmeas, double ratio, double kcmd) {
        k_applied += (kcmd - k_applied) * (1 - std::exp(-dt / 0.12));
        const double dsm = std::isnan(vmeas) ? 0.0 : vmeas * dt;
        speed = std::isnan(vmeas) ? 0.0 : vmeas;
        if (std::fabs(dsm) > 1e-5) move(dsm);
        for (size_t i = 0; i < pts.size();) {
            if (now - pts[i].t > cfg::PT_LIFE) { pts[i] = pts.back(); pts.pop_back(); } else ++i;
        }

        Cmd out;
        if (!active()) { kLast = 0; lastThr = 0; return out; }

        const double frontAge = now - seen[FRONT], sideAge = now - seen[SIDE], backAge = now - seen[BACK];
        if (frontAge > cfg::SONAR_FAULT_FRONT || sideAge > cfg::SONAR_FAULT_OTHER || backAge > cfg::SONAR_FAULT_OTHER)
            return halt(FAULT, "An ultrasonic sensor has been unresponsive too long - stopped.");
        const bool sonarStale = frontAge > cfg::SONAR_CRAWL_AFTER ||
                                sideAge > cfg::SONAR_CRAWL_AFTER || backAge > cfg::SONAR_CRAWL_AFTER;
        if (now - fix.t > 20.0) return halt(FAULT, "GPS fix lost for 20 s - stopped.");
        if (now - fix.t > 5.0) { lastThr = 0; msg = "Waiting for GPS fix..."; return out; }

        const double tdE = tE - posE, tdN = tN - posN;
        dist = std::hypot(tdE, tdN);
        brg = std::atan2(tdE, tdN);
        if (dist < cfg::ARRIVE_RADIUS) {
            char b[96];
            std::snprintf(b, sizeof b, "Arrived: within %.1f m of the target.", cfg::ARRIVE_RADIUS);
            return halt(ARRIVED, b);
        }
        if (hdgKnown) {
            if (st == CAL) { st = NAV; msg = "Heading to target"; }
            const double rel = wrapPi(brg - hdg);
            const double Dl = std::min(dist, 3.0);
            gx = Dl * std::cos(rel); gy = -Dl * std::sin(rel);
        } else { gx = 3.0; gy = 0.0; }

        if (std::fabs(lastThr) > 0.15 && ratio >= 0 && ratio < 0.3) stall += dt; else stall = 0;
        if (stall > 1.5) {
            stall = 0;
            bump(lastThr > 0 ? 1 : -1, now);
            if (st == RECOVER && lastThr < 0) return halt(STUCK, "Blocked behind as well - stopped.");
            return beginRecover(now, "Bumped something - backing up");
        }

        if (st == RECOVER) {
            revDone += -dsm;
            Plan p = planReverse();
            if (!p.ok) {
                if (revDone < 0.15) return halt(STUCK, "Boxed in: nothing is clear ahead or behind - stopped.");
                st = hdgKnown ? NAV : CAL; kLast = 0; msg = "Heading to target";
            } else if (revDone >= cfg::REV_DIST || now - recStart > 6.0) {
                st = hdgKnown ? NAV : CAL; kLast = 0; msg = "Heading to target";
            } else {
                kLast = p.k;
                out.thr = sonarStale ? -cfg::AUTO_THR_MIN : -cfg::REV_THR; lastThr = out.thr;
                out.s = clampd(-p.k / K_MAX, -1, 1);
                out.pan = sweep(now, 0.0);
                return out;
            }
        }

        Plan p = planForward(gx, gy, kLast);
        bool squeeze = false;
        {
            if (!p.ok || p.free < 0.5) {
                const Plan q = planForward(gx, gy, kLast, true);
                if (q.ok && std::fabs(q.k) <= 0.3 * K_MAX && q.free >= std::max(cfg::SQ_MIN, (p.ok ? p.free : 0.0) + 0.2)) { p = q; squeeze = true; }
            }
        }
        if (!p.ok) return beginRecover(now, "Path blocked - backing up");
        if (squeeze) msg = "Squeezing through a narrow gap (slow)";
        else if (msg[0] == 'S' && msg[1] == 'q') msg = "Heading to target";

        const double kslew = cfg::K_SLEW * K_MAX * dt;
        const double k = kLast + clampd(p.k - kLast, -kslew, kslew);
        kLast = k;

        const double vf = clampd((p.free - cfg::S_MIN) / 0.9, 0.0, 1.0);
        double thr = cfg::AUTO_THR_MIN + (cfg::AUTO_THR_MAX - cfg::AUTO_THR_MIN) * vf;
        thr *= 1.0 - 0.4 * std::fabs(k) / K_MAX;
        if (p.clear < 0.06 || squeeze) thr = std::min(thr, cfg::AUTO_THR_MIN);
        if (!hdgKnown) thr = std::min(thr, cfg::CAL_THR);
        if (sonarStale) thr = std::min(thr, cfg::AUTO_THR_MIN);
        out.thr = lastThr = thr;
        out.s = clampd(-k / K_MAX, -1, 1);
        out.pan = sweep(now, clampd(deg(k * 1.0), -15, 15));
        return out;
    }

    std::string json(double now) const {
        std::string j;
        char b[320];
        j.reserve(2048);
        std::snprintf(b, sizeof b, "{\"st\":\"%s\",\"m\":\"%s\",\"gps\":[%d,%.6f,%.6f,%.1f,%.0f],",
                      stName(st), msg.c_str(), gpsMode, fix.lat, fix.lon, gpsHdop,
                      fix.ok ? std::min(now - fix.t, 999.0) : -1.0);
        j += b;
        std::snprintf(b, sizeof b, "\"d\":%.1f,\"b\":%.0f,\"h\":%.0f,\"hk\":%d,\"v\":%.2f,\"pan\":%.0f,",
                      active() || st == ARRIVED ? dist : -1.0, deg(brg < 0 ? brg + 2 * kPi : brg),
                      deg(hdg < 0 ? hdg + 2 * kPi : hdg), hdgKnown ? 1 : 0, speed, deg(rngAng[FRONT]));
        j += b;
        auto cm = [&](int i) { return rng[i] < 0 ? -1 : (rng[i] > 50 ? 999 : (int)std::lround(rng[i] * 100)); };
        std::snprintf(b, sizeof b, "\"r\":[%d,%d,%d],\"g\":[%d,%d],\"p\":[", cm(0), cm(1), cm(2),
                      (int)std::lround(gx * 100), (int)std::lround(gy * 100));
        j += b;
        int n = 0;
        for (const Pt& q : pts) {
            if (n >= 120) break;
            if (q.x < -1.5 || q.x > 3.0 || std::fabs(q.y) > 2.0) continue;
            std::snprintf(b, sizeof b, "%s[%d,%d]", n ? "," : "", (int)std::lround(q.x * 100), (int)std::lround(q.y * 100));
            j += b;
            ++n;
        }
        j += "]}";
        return j;
    }

 private:
    struct Plan { bool ok = false; double k = 0, free = 0, clear = 0, score = -1e9; };

    double kTab[NK], ds = 0.04;
    Pose fw[NK][NS], bw[NK][NS];
    double k_applied = 0, kLast = 0, lastThr = 0, stall = 0;
    double latO = 0, lonO = 0, tE = 0, tN = 0, posE = 0, posN = 0;
    bool havePos = false;
    bool anchorSet = false, reverseFlag = false;
    double aE = 0, aN = 0, aOdom = 0, yawSince = 0, odomTotal = 0;
    double revDone = 0, recStart = 0;
    std::vector<double> recoverTimes;

    static double clampd(double v, double lo, double hi) { return v < lo ? lo : (v > hi ? hi : v); }

    static Pose pose(double k, double u) {
        const double th = k * u;
        Pose p;
        if (std::fabs(k) < 1e-6) { p.x = u; p.y = 0; }
        else { p.x = std::sin(th) / k; p.y = (1 - std::cos(th)) / k; }
        p.c = std::cos(th); p.s = std::sin(th);
        return p;
    }

    void toENU(double lat, double lon, double& e, double& n) const {
        e = (lon - lonO) * 111320.0 * std::cos(rad(latO));
        n = (lat - latO) * 110574.0;
    }

    Cmd halt(State s, const char* why) {
        st = s; msg = why; kLast = 0; lastThr = 0;
        return Cmd{};
    }

    double sweep(double now, double center) const {
        return clampd(center + cfg::SWEEP_AMP * std::sin(2 * kPi * now / cfg::SWEEP_PERIOD),
                      -cfg::PAN_MAX_DEG, cfg::PAN_MAX_DEG);
    }

    void move(double d) {
        const double dth = k_applied * d;
        const double dx = d * std::cos(dth / 2), dy = d * std::sin(dth / 2);
        const double c = std::cos(dth), s = std::sin(dth);
        for (Pt& q : pts) {
            const double px = q.x - dx, py = q.y - dy;
            q.x = px * c + py * s;
            q.y = -px * s + py * c;
        }
        hdg -= dth;
        yawSince += dth;
        odomTotal += d;
        if (d < 0) reverseFlag = true;
    }

    void headingFromGps() {
        if (!anchorSet || reverseFlag) {
            aE = posE; aN = posN; aOdom = odomTotal; yawSince = 0; anchorSet = true; reverseFlag = false;
            return;
        }
        const double trav = odomTotal - aOdom;
        if (trav < cfg::HDG_BASELINE) return;
        const double dE = posE - aE, dN = posN - aN;
        if (std::hypot(dE, dN) > 0.5 * trav && std::fabs(yawSince) < rad(60)) {
            const double hm = std::atan2(dE, dN) - yawSince / 2;
            if (!hdgKnown) { hdg = hm; hdgKnown = true; }
            else hdg += 0.6 * wrapPi(hm - hdg);
        }
        aE = posE; aN = posN; aOdom = odomTotal; yawSince = 0;
    }

    double arcFree(const Pose* P, int n, double& clear, double hm = cfg::HARD_MARGIN, bool sq = false) const {
        const double hl = cfg::RV_HALF_LEN, hw = cfg::RV_HALF_WID;
        double gap = 1e9;
        for (int j = 0; j < n; j++) {
            const Pose& p = P[j];
            for (const Pt& q : pts) {
                const double dx = q.x - p.x, dy = q.y - p.y;
                const double lx = dx * p.c + dy * p.s, ly = -dx * p.s + dy * p.c;
                const double ox = std::fabs(lx) - hl, oy = std::fabs(ly) - hw;
                const double c = sq ? q.u : 0.0;
                if (ox < hm - c && oy < hm - c) {
                    clear = 0;
                    return j == 0 ? 0.0 : (j - 1) * ds;
                }
                const double g = std::max(ox, oy);
                if (g < gap) gap = g;
            }
        }
        clear = gap;
        return (n - 1) * ds;
    }

    Plan planForward(double gxl, double gyl, double kPrev, bool sq = false) const {
        Plan best;
        const double D = std::hypot(gxl, gyl);
        for (int i = 0; i < NK; i++) {
            double clear;
            const double fr = arcFree(fw[i], NS, clear, cfg::HARD_MARGIN, sq);
            if (fr < (sq ? cfg::SQ_MIN : cfg::S_MIN)) continue;
            const double se = std::min(fr, cfg::L_LOOK);
            const int j = std::min(NS - 1, (int)std::lround(se / ds));
            const Pose& e = fw[i][j];
            const double dAfter = std::hypot(gxl - e.x, gyl - e.y);
            const double herr = std::fabs(wrapPi(std::atan2(gyl - e.y, gxl - e.x) - kTab[i] * se));
            const double sc = (D - dAfter) / cfg::L_LOOK
                            + se / cfg::L_LOOK
                            + 0.4 * std::min(clear, 0.25) / 0.25
                            - 0.5 * herr / kPi
                            - 0.15 * std::fabs(kTab[i] - kPrev) / (2 * K_MAX)
                            - (sq ? 0.6 * std::fabs(kTab[i]) / K_MAX : 0.0);
            if (sc > best.score) { best.ok = true; best.k = kTab[i]; best.free = fr; best.clear = clear; best.score = sc; }
        }
        return best;
    }

    Plan planReverse(bool sq = false) const {
        Plan best;
        const int n = std::min(NS, (int)(cfg::L_REV / ds) + 1);
        for (int i = 0; i < NK; i++) {
            double clear;
            const double fr = arcFree(bw[i], n, clear, cfg::HARD_MARGIN + cfg::REV_EXTRA_MARGIN, sq);
            if (fr < cfg::REV_MIN) continue;
            const double sc = fr / cfg::L_REV + 0.4 * std::min(clear, 0.25) / 0.25
                            - 0.15 * std::fabs(kTab[i]) / K_MAX;
            if (sc > best.score) { best.ok = true; best.k = kTab[i]; best.free = fr; best.clear = clear; best.score = sc; }
        }
        return best;
    }

    void bump(int dir, double now) {
        const double x = dir > 0 ? cfg::RV_HALF_LEN + 0.05 : -cfg::RV_HALF_LEN - 0.05;
        for (int d = -1; d <= 1; d++) pts.push_back(Pt{x, 0.12 * d, now + 4.0, FRONT});
    }

    Cmd beginRecover(double now, const char* why) {
        recoverTimes.push_back(now);
        recoverTimes.erase(std::remove_if(recoverTimes.begin(), recoverTimes.end(),
                                          [&](double t) { return now - t > 90.0; }), recoverTimes.end());
        if (recoverTimes.size() > 5) return halt(STUCK, "Keeps getting blocked - gave up. Move it by hand or pick a clearer route.");
        st = RECOVER; revDone = 0; recStart = now; kLast = 0; msg = why;
        return Cmd{};
    }
};
}

static au::Engine g_eng;
static std::mutex g_engM;
static std::atomic<bool> g_sonarOk{false};

static std::atomic<int> g_sonarPulse[3]{};
static std::atomic<double> g_lastPoll{0.0};
static double g_linkTimeout = cfg::LINK_TIMEOUT;

static double secs() { return std::chrono::duration<double>(Clock::now().time_since_epoch()).count(); }

static std::string nmActive(const std::string& iface) {
    std::string cur;
    runCmd({"nmcli", "-g", "GENERAL.CONNECTION", "device", "show", iface}, &cur);
    while (!cur.empty() && (cur.back() == '\n' || cur.back() == '\r')) cur.pop_back();
    return cur;
}
static std::string gatewayOf(const std::string& iface) {
    std::string out;
    runCmd({"ip", "-4", "route", "show", "default", "dev", iface}, &out);
    const size_t p = out.find("via ");
    if (p == std::string::npos) return "";
    const size_t e = out.find_first_of(" \n", p + 4);
    return out.substr(p + 4, e == std::string::npos ? std::string::npos : e - p - 4);
}

static int homeInRange(const std::string& iface, const std::string& home) {
    std::string ssid;
    runCmd({"nmcli", "-g", "802-11-wireless.ssid", "connection", "show", "id", home}, &ssid);
    while (!ssid.empty() && (ssid.back() == '\n' || ssid.back() == '\r')) ssid.pop_back();
    if (ssid.empty()) return -1;
    std::string list;
    runCmd({"nmcli", "-t", "-f", "SSID", "device", "wifi", "list", "ifname", iface, "--rescan", "yes"}, &list);
    if (list.find_first_not_of(" \n\r") == std::string::npos) return -1;
    return list.find(ssid + "\n") != std::string::npos || list.compare(0, ssid.size() + 1, ssid + "\n") == 0 ? 1 : 0;
}
static bool nmUp(const std::string& conn, int waitS) {
    return runCmd({"nmcli", "-w", std::to_string(waitS), "connection", "up", "id", conn}) == 0;
}

static void netThread(std::string iface, std::string home, std::string hotspot, double retryS) {
    int mode = 0, gwFail = 0;
    double lastTry = secs();
    unsigned n = 0;
    auto publish = [&](int m) {
        const Net nn = netFind(iface, 0);
        { std::lock_guard<std::mutex> lk(g_netM); g_net = nn; }
        g_netMode = m;
    };
    auto toHotspot = [&](const char* why) {
        std::fprintf(stderr, "[net] %s - starting hotspot '%s'\n", why, hotspot.c_str());
        g_netMode = 0;
        if (!nmUp(hotspot, 25)) std::fprintf(stderr, "[net] could not start hotspot '%s'\n", hotspot.c_str());
    };
    while (g_run) {
        ++n;
        const std::string cur = nmActive(iface);
        const Net nn = netFind(iface, 0);
        if (cur == home && nn.ok) {
            if (mode != 1) {
                mode = 1; gwFail = 0;
                std::fprintf(stderr, "[net] on home network '%s' (%s). Web control is reachable there.\n",
                             home.c_str(), ipStr(nn.ip).c_str());
            }
            publish(1);
            if (n % 3 == 0) {
                const std::string gw = gatewayOf(iface);
                if (!gw.empty() && runCmd({"ping", "-c", "1", "-W", "1", gw}) != 0) ++gwFail; else gwFail = 0;
                if (gwFail >= 3) { mode = 0; gwFail = 0; toHotspot("home network stopped answering"); }
            }
        } else if (mode == 1) {
            mode = 0;
            toHotspot("lost the home network");
        } else if (cur == hotspot && nn.ok) {
            if (mode != 2) {
                mode = 2; lastTry = secs();
                std::fprintf(stderr, "[net] hotspot '%s' is up (%s)\n", hotspot.c_str(), ipStr(nn.ip).c_str());
            }
            publish(2);
            const bool idle = secs() - g_lastClient.load() > 30.0;
            if (retryS > 0 && idle && secs() - lastTry > retryS) {
                lastTry = secs();
                const int r = homeInRange(iface, home);
                if (r != 0) {
                    std::fprintf(stderr, "[net] trying home network '%s'...\n", home.c_str());
                    if (!nmUp(home, 25)) toHotspot("home network not available");
                }
            }
        } else if (cur == hotspot) {
            g_netMode = 0;
        } else {
            g_netMode = 0;
            std::fprintf(stderr, "[net] joining home network '%s'...\n", home.c_str());
            if (!nmUp(home, 25)) toHotspot("home network not available");
            mode = 0;
        }
        for (int i = 0; i < 20 && g_run; i++) napMs(100);
    }
}

static std::mutex g_authM;
static std::map<std::string, std::pair<int, double>> g_authFail;

static std::string b64dec(const std::string& in) {
    static const std::string T = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int v = 0, b = -8;
    for (unsigned char c : in) {
        const size_t p = T.find((char)c);
        if (p == std::string::npos) break;
        v = (v << 6) + (int)p; b += 6;
        if (b >= 0) { out.push_back((char)((v >> b) & 0xFF)); b -= 8; }
    }
    return out;
}
static bool passOk(const std::string& hdr, const std::string& ip) {
    if (g_password.empty() || hdr.compare(0, 6, "Basic ") != 0) return false;
    const std::string up = b64dec(hdr.substr(6));
    const size_t c = up.find(':');
    const std::string pw = c == std::string::npos ? up : up.substr(c + 1);
    unsigned char d = pw.size() != g_password.size();
    for (size_t i = 0; i < pw.size() && i < g_password.size(); i++) d |= (unsigned char)(pw[i] ^ g_password[i]);
    std::lock_guard<std::mutex> lk(g_authM);
    auto& f = g_authFail[ip];
    if (secs() < f.second) return false;
    if (d == 0) { f = {0, 0}; return true; }
    if (++f.first >= 8) { f = {0, secs() + 600}; std::fprintf(stderr, "[web] too many wrong passwords from %s: blocked 10 min\n", ip.c_str()); }
    return false;
}

static uint64_t monoNs() {
    timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec;
}

static bool headerChip(const char* label) {
    return std::strstr(label, "pinctrl-rp1") || std::strstr(label, "pinctrl-bcm");
}

static int openHeaderChip(int requested, int& chosen) {
    const int first = requested >= 0 ? requested : 0;
    const int last  = requested >= 0 ? requested : 31;
    for (int n = first; n <= last; ++n) {
        char path[48];
        std::snprintf(path, sizeof path, "/dev/gpiochip%d", n);
        const int chip = ::open(path, O_RDWR | O_CLOEXEC);
        if (chip < 0) {
            if (requested >= 0 || errno != ENOENT)
                std::fprintf(stderr, "[sonar] cannot open %s: %s (check GPIO permissions)\n",
                             path, std::strerror(errno));
            continue;
        }
        gpiochip_info info{};
        if (ioctl(chip, GPIO_GET_CHIPINFO_IOCTL, &info) < 0) {
            std::fprintf(stderr, "[sonar] cannot inspect %s: %s\n", path, std::strerror(errno));
            ::close(chip);
            continue;
        }
        if (requested < 0 && (!headerChip(info.label) || info.lines <= (unsigned)cfg::BACK_ECHO)) {
            ::close(chip);
            continue;
        }
        if (requested >= 0 && !headerChip(info.label))
            std::fprintf(stderr, "[sonar] WARNING: %s is labelled '%s', not a known Pi header GPIO chip\n",
                         path, info.label);
        chosen = n;
        std::fprintf(stderr, "[sonar] using %s (label '%s', %u lines)\n", path, info.label, info.lines);
        return chip;
    }
    if (requested < 0)
        std::fprintf(stderr, "[sonar] could not find the 40-pin header GPIO controller; "
                             "check 'gpiodetect' and use --gpiochip N if necessary.\n");
    return -1;
}

struct SonarTrace {
    double riseMs = -1.0, widthMs = -1.0;
    int n = 0;
    double t[12] = {};
    bool up[12] = {};
};

struct Sonar {
    int tfd = -1, efd = -1;
    ~Sonar() { if (tfd >= 0) ::close(tfd); if (efd >= 0) ::close(efd); }

    static int request(int chip, unsigned off, bool out) {
        gpio_v2_line_request rq{};
        rq.offsets[0] = off;
        rq.num_lines = 1;
        std::strncpy(rq.consumer, "rover", sizeof rq.consumer - 1);
        rq.config.flags = out ? GPIO_V2_LINE_FLAG_OUTPUT
                              : (GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_EDGE_RISING | GPIO_V2_LINE_FLAG_EDGE_FALLING |
                                 (cfg::SONAR_ECHO_PULLUP ? GPIO_V2_LINE_FLAG_BIAS_PULL_UP : 0));
        if (ioctl(chip, GPIO_V2_GET_LINE_IOCTL, &rq) < 0) return -1;
        return rq.fd;
    }
    bool open(int chip, unsigned trig, unsigned echo) {
        tfd = request(chip, trig, true);
        if (tfd < 0) return false;
        efd = request(chip, echo, false);
        return efd >= 0;
    }
    bool setTrig(bool on) const {
        gpio_v2_line_values v{};
        v.mask = 1;
        v.bits = on ? 1 : 0;
        return ioctl(tfd, GPIO_V2_LINE_SET_VALUES_IOCTL, &v) == 0;
    }
    static int decodePulse(uint64_t rise, uint64_t fall, double& range) {
        if (!rise || fall <= rise) return -4;
        const double d = (double)(fall - rise) * 1e-9 * 343.0 / 2.0;
        if (d < 0.005) return -4;
        if (d > cfg::SONAR_MAX) return 2;
        range = d;
        return 1;
    }
    int measure(double& range, SonarTrace* trace = nullptr) const {
        if (trace) *trace = SonarTrace{};
        gpio_v2_line_event ev{};
        pollfd p{efd, POLLIN, 0};
        while (true) {
            const int ready = poll(&p, 1, 0);
            if (ready == 0) break;
            if (ready < 0) { if (errno == EINTR) continue; return -3; }
            if (p.revents & (POLLERR | POLLNVAL | POLLHUP)) return -3;
            if (read(efd, &ev, sizeof ev) != (ssize_t)sizeof ev) return -3;
        }
        gpio_v2_line_values levels{};
        levels.mask = 1;
        if (ioctl(efd, GPIO_V2_LINE_GET_VALUES_IOCTL, &levels) < 0) return -3;
        if (levels.bits & 1) {
            const uint64_t t0 = monoNs();
            bool low = false;
            while (monoNs() - t0 < 60000000ull) {
                timespec ts{0, 5000000};
                const int ready = ppoll(&p, 1, &ts, nullptr);
                if (ready > 0) {
                    if (read(efd, &ev, sizeof ev) != (ssize_t)sizeof ev) return -3;
                    if (ev.id == GPIO_V2_LINE_EVENT_FALLING_EDGE) { low = true; break; }
                } else if (ready < 0 && errno != EINTR) return -3;
                if (ioctl(efd, GPIO_V2_LINE_GET_VALUES_IOCTL, &levels) < 0) return -3;
                if (!(levels.bits & 1)) { low = true; break; }
            }
            if (!low) return -5;
            while (poll(&p, 1, 0) > 0) { if (read(efd, &ev, sizeof ev) != (ssize_t)sizeof ev) break; }
        }

        if (!setTrig(true)) return -3;
        const uint64_t highStart = monoNs();
        const uint64_t highNs = (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
                                    cfg::SONAR_TRIGGER_HIGH).count();
        while (monoNs() - highStart < highNs) {}
        if (!setTrig(false)) return -3;

        uint64_t rise = 0;
        const uint64_t start = monoNs();
        while (true) {
            const uint64_t now = monoNs();
            const uint64_t limit = (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
                rise ? cfg::SONAR_FALL_WAIT : cfg::SONAR_RISE_WAIT).count();
            const uint64_t from = rise ? rise : start;
            const uint64_t el = now >= from ? now - from : 0;
            if (el >= limit) return rise ? -2 : 3;
            timespec ts{(time_t)((limit - el) / 1000000000ull),
                        (long)((limit - el) % 1000000000ull)};
            const int ready = ppoll(&p, 1, &ts, nullptr);
            if (ready == 0) continue;
            if (ready < 0) { if (errno == EINTR) continue; return -3; }
            if (p.revents & (POLLERR | POLLNVAL | POLLHUP)) return -3;
            if (read(efd, &ev, sizeof ev) != (ssize_t)sizeof ev) return -3;
            if (trace && trace->n < 12) {
                trace->t[trace->n] = (double)(ev.timestamp_ns >= start ? ev.timestamp_ns - start : 0) / 1e6;
                trace->up[trace->n] = ev.id == GPIO_V2_LINE_EVENT_RISING_EDGE;
                ++trace->n;
            }
            if (ev.id == GPIO_V2_LINE_EVENT_RISING_EDGE) {
                if (!rise) {
                    rise = ev.timestamp_ns;
                    if (trace) trace->riseMs = (double)(rise >= start ? rise - start : 0) / 1e6;
                }
            } else if (ev.id == GPIO_V2_LINE_EVENT_FALLING_EDGE && rise) {
                if (trace) trace->widthMs = (double)(ev.timestamp_ns - rise) / 1e6;
                return decodePulse(rise, ev.timestamp_ns, range);
            }
        }
    }
};

static const char* sonarResultName(int result) {
    switch (result) {
        case 1: return "distance received";
        case 2: return "beyond 4.5 m (valid echo pulse)";
        case 3: return "no echo (nothing in range)";
        case -1: return "no rising echo edge";
        case -2: return "echo stayed high / no falling edge";
        case -3: return "GPIO read/trigger error";
        case -4: return "invalid echo pulse";
        case -5: return "ECHO reads HIGH while idle - ECHO wire unplugged/loose (or module does not drive ECHO low)";
        default: return "not yet read";
    }
}

static void sonarThread(int chipN) {
    int chosen = -1;
    const int chip = openHeaderChip(chipN, chosen);
    if (chip < 0) return;
    Sonar sn[3];
    const int pins[3][2] = {{cfg::FRONT_TRIG, cfg::FRONT_ECHO},
                            {cfg::SIDE_TRIG, cfg::SIDE_ECHO},
                            {cfg::BACK_TRIG, cfg::BACK_ECHO}};
    const char* names[3] = {"front", "side", "back"};
    for (int i = 0; i < 3; ++i) {
        if (!sn[i].open(chip, (unsigned)pins[i][0], (unsigned)pins[i][1])) {
            std::fprintf(stderr, "[sonar] GPIO request failed on chip %d for %s TRIG=%d/ECHO=%d: %s. "
                                 "Check the selected chip and whether another process owns those lines.\n",
                         chosen, names[i], pins[i][0], pins[i][1], std::strerror(errno));
            ::close(chip);
            return;
        }
    }
    ::close(chip);
    g_sonarOk = true;
    for (int i = 0; i < 3; ++i) {
        gpio_v2_line_values lv{};
        lv.mask = 1;
        if (ioctl(sn[i].efd, GPIO_V2_LINE_GET_VALUES_IOCTL, &lv) == 0 && (lv.bits & 1))
            std::fprintf(stderr, "[sonar] WARNING: %s ECHO pin reads HIGH at idle: check that its ECHO wire is connected%s\n",
                         names[i], cfg::SONAR_ECHO_PULLUP ? "" : " (pull-up is off)");
    }
    std::fprintf(stderr, "[sonar] GPIO pins claimed; sensors are assumed connected (no startup check)\n");

    const int order[4] = {0, 1, 0, 2};
    int fails[3] = {0, 0, 0};
    bool reported[3] = {false, false, false};
    int quiet[3] = {0, 0, 0};
    bool everEcho[3] = {false, false, false};
    int tally[3][3] = {};
    double tallyT = secs();
    double lastWarning[3] = {-1e9, -1e9, -1e9};
    for (unsigned n = 0; g_run; ++n) {
        const int i = order[n & 3];
        double r = 0;
        const int result = sn[i].measure(r);
        g_sonarPulse[i] = result;
        const double now = secs();
        ++tally[i][(result == 1 || result == 2) ? 0 : result == 3 ? 1 : 2];
        if (now - tallyT >= 5.0) {
            if (tally[0][2] + tally[1][2] + tally[2][2] > 0)
                std::fprintf(stderr, "[sonar] last 5 s  (echo/no-echo/error)  front %d/%d/%d  side %d/%d/%d  back %d/%d/%d\n",
                             tally[0][0], tally[0][1], tally[0][2], tally[1][0], tally[1][1], tally[1][2],
                             tally[2][0], tally[2][1], tally[2][2]);
            std::memset(tally, 0, sizeof tally);
            tallyT = now;
        }
        if (result > 0) {
            fails[i] = 0;
            if (result == 3) {
                if (++quiet[i] == 200 && !everEcho[i])
                    std::fprintf(stderr, "[sonar] %s: has never heard an echo. Normal if nothing is within ~3 m; "
                                         "if something was, check its wiring.\n", names[i]);
            } else { quiet[i] = 0; everEcho[i] = true; }
            if (!reported[i] && result != 3) {
                std::fprintf(stderr, "[sonar] %s: first valid pulse (%s)\n", names[i], sonarResultName(result));
                reported[i] = true;
            }
            const double ang = i == 0 ? au::rad(g_panDeg.load()) : au::MOUNT[i].ang;
            std::lock_guard<std::mutex> lk(g_engM);
            g_eng.addRange(i, result == 1, r, ang, now);
        } else if (++fails[i] >= 6 && now - lastWarning[i] > 10.0) {
            std::fprintf(stderr, "[sonar] %s: %s (%d in a row)\n", names[i], sonarResultName(result), fails[i]);
            lastWarning[i] = now;
        }
        std::this_thread::sleep_for(cfg::SONAR_INTER_PING);
    }
    g_sonarOk = false;
}

static int sonarTestMode(int chipN, bool rawEdges) {
    int chosen = -1;
    const int chip = openHeaderChip(chipN, chosen);
    if (chip < 0) return 1;
    Sonar sn[3];
    const int pins[3][2] = {{cfg::FRONT_TRIG, cfg::FRONT_ECHO},
                            {cfg::SIDE_TRIG, cfg::SIDE_ECHO},
                            {cfg::BACK_TRIG, cfg::BACK_ECHO}};
    const char* names[3] = {"front", "side", "back"};
    for (int i = 0; i < 3; ++i) {
        if (!sn[i].open(chip, (unsigned)pins[i][0], (unsigned)pins[i][1])) {
            std::fprintf(stderr, "[sonar-test] cannot claim %s pins TRIG=%d ECHO=%d: %s\n",
                         names[i], pins[i][0], pins[i][1], std::strerror(errno));
            ::close(chip);
            return 1;
        }
    }
    ::close(chip);
    std::fprintf(stderr, "[sonar-test] pull-up on ECHO pins: %s; Ctrl+C exits\n", cfg::SONAR_ECHO_PULLUP ? "ON" : "off");
    for (int i = 0; i < 3; ++i) {
        gpio_v2_line_values lv{};
        lv.mask = 1;
        const bool hi = ioctl(sn[i].efd, GPIO_V2_LINE_GET_VALUES_IOCTL, &lv) == 0 && (lv.bits & 1);
        std::fprintf(stderr, "[sonar-test] %-5s ECHO idle level: %s%s\n", names[i], hi ? "HIGH" : "low",
                     hi ? "   <-- unplugged, or this module does not drive ECHO low" : "");
    }
    const double testStart = secs();
    while (g_run) {
        for (int i = 0; i < 3 && g_run; ++i) {
            double r = 0;
            SonarTrace trace;
            const int rc = sn[i].measure(r, &trace);
            if (rc == 1)
                std::printf("[sonar-test] %6.2fs %-5s %.3f m (rise %.2f ms; echo %.2f ms)\n",
                            secs() - testStart, names[i], r, trace.riseMs, trace.widthMs);
            else if (rc == 2)
                std::printf("[sonar-test] %6.2fs %-5s %s (echo %.2f ms)\n",
                            secs() - testStart, names[i], sonarResultName(rc), trace.widthMs);
            else
                std::printf("[sonar-test] %6.2fs %-5s %s\n",
                            secs() - testStart, names[i], sonarResultName(rc));
            if (rawEdges) {
                std::printf("               edges after trigger:");
                for (int e = 0; e < trace.n; ++e) std::printf(" %s@%.2fms", trace.up[e] ? "UP" : "DOWN", trace.t[e]);
                if (!trace.n) std::printf(" none");
                std::printf("\n");
            }
            std::fflush(stdout);
            std::this_thread::sleep_for(cfg::SONAR_INTER_PING);
        }
    }
    return 0;
}

static bool jnum(const std::string& s, const char* key, double& v) {
    const std::string k = std::string("\"") + key + "\":";
    const size_t p = s.find(k);
    if (p == std::string::npos) return false;
    v = std::strtod(s.c_str() + p + k.size(), nullptr);
    return true;
}

static void gpsThread() {
    double hdop = 99;
    bool warned = false;
    while (g_run) {
        const int fd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_port = htons(2947);
        inet_pton(AF_INET, "127.0.0.1", &a.sin_addr);
        if (fd < 0 || connect(fd, (sockaddr*)&a, sizeof a) != 0) {
            if (fd >= 0) close(fd);
            if (!warned) std::fprintf(stderr, "[gps] cannot reach gpsd on 127.0.0.1:2947 (is gpsd running?) - retrying\n");
            warned = true;
            for (int i = 0; i < 20 && g_run; i++) napMs(100);
            continue;
        }
        warned = false;
        const char* w = "?WATCH={\"enable\":true,\"json\":true}\n";
        send(fd, w, std::strlen(w), MSG_NOSIGNAL);
        std::string buf;
        char tmp[2048];
        while (g_run) {
            pollfd p{fd, POLLIN, 0};
            if (poll(&p, 1, 500) <= 0) continue;
            const ssize_t n = recv(fd, tmp, sizeof tmp, 0);
            if (n <= 0) break;
            buf.append(tmp, (size_t)n);
            size_t nl;
            while ((nl = buf.find('\n')) != std::string::npos) {
                const std::string line = buf.substr(0, nl);
                buf.erase(0, nl + 1);
                if (line.find("\"class\":\"SKY\"") != std::string::npos) {
                    double h;
                    if (jnum(line, "hdop", h)) hdop = h;
                } else if (line.find("\"class\":\"TPV\"") != std::string::npos) {
                    double mode = 0, lat, lon, eph = -1;
                    jnum(line, "mode", mode);
                    jnum(line, "eph", eph);
                    const bool pos = mode >= 2 && jnum(line, "lat", lat) && jnum(line, "lon", lon);
                    std::lock_guard<std::mutex> lk(g_engM);
                    g_eng.noteGps((int)mode, hdop);
                    if (pos && (hdop <= cfg::HDOP_MAX || (eph > 0 && eph <= 8.0))) {
                        au::Fix f;
                        f.ok = true; f.lat = lat; f.lon = lon; f.hdop = hdop; f.t = secs();
                        g_eng.addGps(f);
                    }
                }
            }
            if (buf.size() > 65536) buf.clear();
        }
        close(fd);
    }
}

static void autoThread() {
    auto next = Clock::now();
    double tPrev = secs();
    while (g_run) {
        next += 50ms;
        std::this_thread::sleep_until(next);
        const double now = secs();
        const double dt = std::min(std::max(now - tPrev, 0.005), 0.2);
        tPrev = now;

        au::Cmd c;
        bool act;
        std::string js;
        {
            std::lock_guard<std::mutex> lk(g_engM);
            if (g_estop && g_eng.active()) g_eng.cancel("E-STOP pressed");
            if (g_autoCancel.exchange(false) && g_eng.active()) g_eng.cancel("Cancelled: manual control took over");
            if (g_linkTimeout > 0 && g_eng.active() && now - g_lastPoll > g_linkTimeout)
                g_eng.cancel("Stopped: lost contact with the control page");
            c = g_eng.step(now, dt, g_vbody, g_ratio, g_kcmd);
            act = g_eng.active();
            js = g_eng.json(now);
        }
        {
            std::lock_guard<std::mutex> lk(g_auto.m);
            g_auto.cmd = AutoCmd{act, c.thr, c.s, c.pan};
        }
        js.insert(1, std::string("\"nm\":") + std::to_string(g_netMode.load()) + ",\"es\":" + (g_estop ? "1" : "0") + ",\"so\":" + (g_sonarOk ? "1" : "0") +
                  ",\"ss\":[" + std::to_string(g_sonarPulse[0].load()) + "," +
                  std::to_string(g_sonarPulse[1].load()) + "," +
                  std::to_string(g_sonarPulse[2].load()) + "],");
        std::lock_guard<std::mutex> lk(g_statusM);
        g_status = std::move(js);
    }
}

static const char* PAGE = R"HTML(<!doctype html><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1">
<title>Rover</title>
<style>
*{-webkit-user-select:none;user-select:none;-webkit-touch-callout:none;-webkit-tap-highlight-color:transparent}
input{-webkit-user-select:text;user-select:text}
body{margin:0;background:#111;color:#bbb;font:14px system-ui,sans-serif;text-align:center;touch-action:manipulation}
header{display:flex;gap:6px;padding:6px}
button{border:0;border-radius:8px;background:#272727;color:#bbb;font-size:18px;touch-action:none}
header button{flex:1;height:40px;font-size:15px}
#es{flex:0 0 auto;padding:0 16px;background:#c22;color:#fff;font-weight:700}
#es.r{background:#e90;color:#000}
.on{background:#46f;color:#fff}
.hide{display:none!important}
#v{display:block;width:100%;height:48vh;object-fit:contain;background:#000;pointer-events:none}
#p1{display:flex;flex-wrap:wrap;justify-content:center;align-items:flex-start;gap:20px;padding:12px}
#p1 div{display:flex;flex-direction:column;align-items:center;gap:6px}
.g{display:grid;grid:repeat(3,50px)/repeat(3,50px);gap:5px}
#p2{display:flex;flex-direction:column;align-items:center;gap:8px;padding:12px}
#p2 input{width:min(320px,90vw);padding:9px;border:1px solid #333;border-radius:8px;background:#1b1b1b;color:#ddd;font-size:16px}
#p2 .r2{display:flex;gap:8px}
#p2 .r2 button{width:130px;height:40px;font-size:16px}
#go{background:#2a7}#go:disabled{opacity:.4}
#st{font-size:16px;color:#fff}
pre{margin:0;font:12px/1.5 ui-monospace,monospace;text-align:left;white-space:pre-wrap}
canvas{background:#000;border-radius:8px}
small{opacity:.6}
</style>
<header><button id=t1 class=on>Remote control</button><button id=t2>Autonomous</button><button id=es>E-STOP</button></header>
<img id=v alt="">
<main id=p1>
<div><small>DRIVE</small><div class=g>
<i></i><button data-k=w>W</button><i></i>
<button data-k=a>A</button><button data-k=" ">&#9679;</button><button data-k=d>D</button>
<i></i><button data-k=s>S</button><i></i></div></div>
<div><small>SPEED <output id=o>50</output>%</small><input id=s type=range min=10 max=100 value=50>
<button id=b style="width:100px;height:36px;font-size:14px">Video</button><small id=t>&#9679;</small></div>
<div><small>CAMERA</small><div class=g>
<i></i><button data-k=ArrowUp>&#9650;</button><i></i>
<button data-k=ArrowLeft>&#9664;</button><button data-k=c>&#9679;</button><button data-k=ArrowRight>&#9654;</button>
<i></i><button data-k=ArrowDown>&#9660;</button><i></i></div></div>
<small style="width:100%">WASD drive &middot; Arrows camera &middot; Space centre wheels &middot; C centre camera &middot; Esc E-STOP</small>
</main>
<section id=p2 class=hide>
<input id=la type=text inputmode=decimal autocomplete=off placeholder="Target latitude  e.g. 32.715700">
<input id=lo type=text inputmode=decimal autocomplete=off placeholder="Target longitude  e.g. -117.161100">
<input id=hh type=text inputmode=decimal autocomplete=off placeholder="Rover faces (deg from north) - blank = find out">
<div class=r2><button id=go>GO</button><button id=cx>Cancel</button></div>
<b id=st>IDLE</b><small id=ms></small>
<pre id=info></pre>
<canvas id=rd width=240 height=220></canvas>
<small>Radar: rover (blue) faces up. Dots = remembered echoes, line = way to the target.<br>Esc or the red button stops everything. The page must stay open: the rover stops if it loses contact.</small>
</section>
<script>
const $=x=>document.getElementById(x),K={w:1,s:2,a:4,d:8,ArrowUp:16,ArrowDown:32,ArrowLeft:64,ArrowRight:128},O={' ':256,c:512},
B=[...document.querySelectorAll('button[data-k]')],img=$('v'),
blank='data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==';
let m=0,o=0,l='',t=0,on=1,vis=1,tab=1,es=0,tStop=0,hist=[];
function key(k,d){if(k.length==1)k=k.toLowerCase();
if(k in O){if(d)o|=O[k]}
else if(k in K){m=d?m|K[k]:m&~K[k];B.forEach(b=>b.classList.toggle('on',!!(m&K[b.dataset.k])))}}
function vid(){img.src=on&&vis?'/stream':blank;$('b').style.opacity=on?1:.5}
function show(n){tab=n;m=0;B.forEach(b=>b.classList.remove('on'));
$('p1').classList.toggle('hide',n!=1);$('p2').classList.toggle('hide',n!=2);
$('t1').classList.toggle('on',n==1);$('t2').classList.toggle('on',n==2)}
function paint(){$('es').textContent=es?'RESET':'E-STOP';$('es').classList.toggle('r',!!es)}
function stop(){if(es)return;es=1;tStop=Date.now();paint();
for(let i=0;i<3;i++)setTimeout(()=>fetch('/e').catch(()=>{}),i*150)}
$('es').onpointerdown=e=>{e.preventDefault();stop()};
$('es').onclick=()=>{if(es&&Date.now()-tStop>800){fetch('/r').then(()=>{es=0;paint()},()=>{})}};
$('t1').onclick=()=>show(1);$('t2').onclick=()=>show(2);
addEventListener('keydown',e=>{
if(e.key=='Escape'){e.preventDefault();stop();return}
if(tab!=1||e.target.tagName=='INPUT')return;
if(e.key in K||e.key in O)e.preventDefault();if(!e.repeat)key(e.key,1)});
addEventListener('keyup',e=>key(e.key,0));
addEventListener('contextmenu',e=>e.preventDefault());
addEventListener('blur',()=>{m=0;B.forEach(b=>b.classList.remove('on'))});
B.forEach(b=>{const k=b.dataset.k;b.onpointerdown=e=>{e.preventDefault();b.setPointerCapture(e.pointerId);key(k,1)};b.onpointerup=b.onpointercancel=()=>key(k,0)});
$('s').oninput=()=>$('o').value=$('s').value;
$('b').onclick=()=>{on=!on;vid()};
document.onvisibilitychange=()=>{vis=!document.hidden;vid()};
setInterval(()=>{const v=$('s').value,q=m+','+v,n=Date.now();
if(q!=l||o||(m&&n-t>100)){fetch('/c?k='+(m|o)+'&v='+v).then(()=>$('t').style.color='#4c4',()=>$('t').style.color='#c44');l=q;o=0;t=n}},40);
const cmv=c=>c<0?'--':c>=999?'>4.5 m':c+' cm';
const sonic=s=>s==1?'OK':s==2?'CLEAR >4.5m':s==3?'NO ECHO (clear)':s==0?'WAIT':s==-1?'NO RISE':s==-2?'NO FALL':s==-3?'GPIO ERROR':s==-5?'ECHO HIGH':'BAD PULSE';
function radar(j){const c=$('rd').getContext('2d'),W=240,H=220,cx=120,cy=170,sc=.7;c.clearRect(0,0,W,H);
c.strokeStyle='#222';for(let r=100;r<=300;r+=100){c.beginPath();c.arc(cx,cy,r*sc,Math.PI,2*Math.PI);c.stroke()}
c.fillStyle='#46f';c.fillRect(cx-23*sc,cy-33*sc,46*sc,66*sc);
if(j.hk&&j.g){c.strokeStyle='#2c6';c.beginPath();c.moveTo(cx,cy);c.lineTo(cx-j.g[1]*sc,cy-j.g[0]*sc);c.stroke()}
c.fillStyle='#e55';for(const q of j.p){c.fillRect(cx-q[1]*sc-1,cy-q[0]*sc-1,3,3)}
const a=j.pan*Math.PI/180;c.strokeStyle='#fc3';c.beginPath();c.moveTo(cx,cy-30*sc);c.lineTo(cx-Math.sin(a)*40,cy-30*sc-Math.cos(a)*40);c.stroke()}
function upd(j){
if(j.es&&!es){es=1;paint()}else if(!j.es&&es&&Date.now()-tStop>1500){es=0;paint()}
$('st').textContent=j.es?'E-STOPPED':j.st;$('ms').textContent=j.m||'';
const g=j.gps,fx=g[0]>=3?'3D fix':g[0]==2?'2D fix':'no fix';
$('info').textContent='GPS '+fx+'  hdop '+g[3]+(g[0]>=2?'\n'+g[1].toFixed(6)+', '+g[2].toFixed(6):'')+
(j.d>=0?'\nTarget '+j.d+' m  bearing '+j.b+'°':'')+'\nHeading '+(j.hk?j.h+'°':'unknown')+
'   speed '+j.v+' m/s\nCamera sonar '+cmv(j.r[0])+' @ '+j.pan+'° (+ left) ['+sonic(j.ss[0])+']' +
'\nSide '+cmv(j.r[1])+' ['+sonic(j.ss[1])+']   Back '+cmv(j.r[2])+' ['+sonic(j.ss[2])+']'+
(j.so?'':'\nSONAR GPIO NOT READY — see rover log');
radar(j)}
async function poll(){try{const r=await fetch('/s',{cache:'no-store'});upd(await r.json())}catch(e){}setTimeout(poll,tab==2?500:1500)}
$('go').onclick=async()=>{const la=parseFloat($('la').value),lo=parseFloat($('lo').value),h=$('hh').value.trim();
if(!(la>=-90&&la<=90&&lo>=-180&&lo<=180)){$('ms').textContent='Enter a valid latitude and longitude';return}
if(h!==''&&isNaN(parseFloat(h))){$('ms').textContent='Heading must be a number (0-360) or blank';return}
try{localStorage.setItem('tgt',JSON.stringify([$('la').value,$('lo').value,h]))}catch(e){}
try{const r=await fetch('/a?lat='+la+'&lon='+lo+(h!==''?'&h='+parseFloat(h):''));const x=await r.text();if(x!='OK')$('ms').textContent=x}catch(e){$('ms').textContent='No contact with the rover'}};
$('cx').onclick=()=>fetch('/x').catch(()=>{});
try{const v=JSON.parse(localStorage.getItem('tgt')||'null');if(v){$('la').value=v[0];$('lo').value=v[1];$('hh').value=v[2]}}catch(e){}
vid();paint();poll();
</script>)HTML";

static void usage(const char* p) {
    std::cout << "Usage: " << p << " <serial port> [--iface wlan0 | --bind <ip>] [--hotspot <connection>] [--port 8080]\n"
              << "       [--home <NetworkManager connection, e.g. netplan-wlan0-MuniMann-5G>] [--password-file FILE] [--home-retry S (default 0 = never go back)]\n"
              << "       [--js /dev/input/js0] [--res WxH] [--fps N] [--quality Q] [--no-pad] [--no-web]\n"
              << "       [--no-detect] [--detect-json FILE] [--cam-on-demand]\n"
              << "       [--yolo-model FILE.rpk [--yolo-labels FILE] [--yolo-threshold 0.4] [--cam-script rover_cam.py]]\n"
              << "       [--hq-res WxH (default 640x480)] [--hq-quality Q (default 60)] [--hq-fps N (default 10)] [--no-hq]\n"
              << "         (with --yolo-model and --home: the picture is --res/--quality/--fps on the hotspot, and the --hq-* values on the home network)\n"
              << "       [--no-auto] [--gpiochip N (auto by default)] [--link-timeout SECONDS (0 = off)]\n"
              << "       " << p << " --sonar-test [--gpiochip N]  (test sensors only, no servo bus)\n"
              << "       " << p << " --sonar-raw   (same, and prints every ECHO edge; point a sensor at open space too)\n";
}

int main(int argc, char** argv) {
    std::string serial, js = "/dev/input/js0", iface = "wlan0", bindIp, hotspot, homeConn, passFile;
    double homeRetry = 0;
    int port = 8080, camW = cfg::CAM_W, camH = cfg::CAM_H, camFps = cfg::CAM_FPS, camQ = cfg::CAM_QUALITY;
    bool usePad = true, useWeb = true, useAuto = true, detect = true, camOnDemand = false, sonarTest = false, sonarRaw = false;
    std::string detectJson = cfg::DETECT_JSON;
    std::string yoloModel, camScript, yoloLabels;
    int hqW = 640, hqH = 480, hqQ = 60, hqFps = 10;
    bool useHq = true;
    double yoloThr = 0.4;
    int gpioChip = cfg::GPIOCHIP;
    for (int i = 1; i < argc; i++) {
        const std::string a = argv[i];
        const bool more = i + 1 < argc;
        if (a == "--no-pad") usePad = false;
        else if (a == "--no-web") useWeb = false;
        else if (a == "--no-auto") useAuto = false;
        else if (a == "--sonar-test") sonarTest = true;
        else if (a == "--sonar-raw") { sonarTest = true; sonarRaw = true; }
        else if (a == "--no-detect") detect = false;
        else if (a == "--cam-on-demand") camOnDemand = true;
        else if (a == "--detect-json" && more) detectJson = argv[++i];
        else if (a == "--yolo-model" && more) yoloModel = argv[++i];
        else if (a == "--cam-script" && more) camScript = argv[++i];
        else if (a == "--yolo-labels" && more) yoloLabels = argv[++i];
        else if (a == "--yolo-threshold" && more) yoloThr = std::atof(argv[++i]);
        else if (a == "--no-hq") useHq = false;
        else if (a == "--hq-quality" && more) hqQ = std::atoi(argv[++i]);
        else if (a == "--hq-fps" && more) hqFps = std::atoi(argv[++i]);
        else if (a == "--hq-res" && more) {
            if (std::sscanf(argv[++i], "%dx%d", &hqW, &hqH) != 2 || hqW < 160 || hqH < 120) { usage(argv[0]); return 1; }
        }
        else if (a == "--gpiochip" && more) gpioChip = std::atoi(argv[++i]);
        else if (a == "--link-timeout" && more) g_linkTimeout = std::atof(argv[++i]);
        else if (a == "--port" && more) port = std::atoi(argv[++i]);
        else if (a == "--js" && more) js = argv[++i];
        else if (a == "--iface" && more) iface = argv[++i];
        else if (a == "--bind" && more) bindIp = argv[++i];
        else if (a == "--hotspot" && more) hotspot = argv[++i];
        else if (a == "--home" && more) homeConn = argv[++i];
        else if (a == "--password-file" && more) passFile = argv[++i];
        else if (a == "--home-retry" && more) homeRetry = std::atof(argv[++i]);
        else if (a == "--fps" && more) camFps = std::atoi(argv[++i]);
        else if (a == "--quality" && more) camQ = std::atoi(argv[++i]);
        else if (a == "--res" && more) {
            if (std::sscanf(argv[++i], "%dx%d", &camW, &camH) != 2) { usage(argv[0]); return 1; }
        }
        else if (a == "-h" || a == "--help" || (!a.empty() && a[0] == '-')) { usage(argv[0]); return 1; }
        else serial = a;
    }
    if (serial.empty() && !sonarTest) { usage(argv[0]); return 1; }

    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);
    std::signal(SIGHUP, onSignal);
    std::signal(SIGPIPE, SIG_IGN);

    if (sonarTest) return sonarTestMode(gpioChip, sonarRaw);

    SMS_STS bus;
    if (!bus.begin(1000000, serial.c_str())) {
        std::fprintf(stderr, "Failed to open servo bus on %s\n", serial.c_str());
        return 1;
    }

    double tilt0 = cfg::CAM_HOME, pan0 = cfg::CAM_HOME;
    initServos(bus, tilt0, pan0);

    std::thread ctl(controlLoop, std::ref(bus), tilt0, pan0);
    std::thread pad, cam, sonar, gps, autoT, modeT;
    if (useAuto) {
        g_lastPoll = secs();
        sonar = std::thread(sonarThread, gpioChip);
        gps = std::thread(gpsThread);
        autoT = std::thread(autoThread);
    }
    if (usePad) pad = std::thread(padThread, js);

    if (useWeb) {
        if (!homeConn.empty()) {
            g_multi = true;
            if (hotspot.empty()) hotspot = "RoverNet";
            const char* envPw = std::getenv("ROVER_PASSWORD");
            if (!passFile.empty()) {
                if (FILE* f = std::fopen(passFile.c_str(), "r")) {
                    char b[256] = {0};
                    if (std::fgets(b, sizeof b, f)) g_password = b;
                    std::fclose(f);
                }
            } else if (envPw) g_password = envPw;
            while (!g_password.empty() && (g_password.back() == '\n' || g_password.back() == '\r')) g_password.pop_back();
            if (g_password.size() < 8)
                std::fprintf(stderr, "[web] NO PASSWORD (or shorter than 8 characters): home-network / internet access is "
                                     "REFUSED. Set one in --password-file or ROVER_PASSWORD. The hotspot still works without it.\n");
            else std::fprintf(stderr, "[web] remote access needs the web password (user name: anything)\n");
            g_net.ok = true;
        } else
        g_net = resolveNet(iface, bindIp, hotspot);
        if (!g_net.ok && g_run) {
            std::fprintf(stderr, "[web] Pi network not available - web control DISABLED.\n"
                                 "[web] IPv4 addresses on this Pi:\n");
            listIfaces();
            std::fprintf(stderr, "[web] Use --iface <name>, --bind <ip> or --hotspot <connection>.\n");
            useWeb = false;
        }
    }

    if (useWeb) {
        std::string detectArg;
        if (detect) {
            if (access(detectJson.c_str(), R_OK) == 0) {
                detectArg = " --post-process-file '" + detectJson + "'";
                g_camAlways = !camOnDemand;
                std::fprintf(stderr, "[cam] object detection ON (%s)%s\n", detectJson.c_str(),
                             g_camAlways ? "; camera stays running. First start can take ~1 min." : "");
            } else {
                std::fprintf(stderr, "[cam] object-detection file not found: %s\n"
                                     "[cam] install it with: sudo apt install imx500-all   (streaming WITHOUT labels for now)\n",
                             detectJson.c_str());
            }
        }
        char cmd[768];
        std::snprintf(cmd, sizeof cmd,
            "B=$(command -v rpicam-vid || command -v libcamera-vid); exec \"$B\" -t 0 -n --codec mjpeg "
            "--width %d --height %d --framerate %d --quality %d --rotation 180 --flush%s -o - 2>/dev/null",
            camW, camH, camFps, camQ, detectArg.c_str());
        std::string camCmd = cmd;
        if (!yoloModel.empty() && detect) {
            if (camScript.empty()) {
                char exe[1024] = {0};
                const ssize_t n = readlink("/proc/self/exe", exe, sizeof exe - 1);
                std::string dir = n > 0 ? std::string(exe, (size_t)n) : std::string(argv[0]);
                const size_t sl = dir.rfind('/');
                camScript = (sl == std::string::npos ? std::string(".") : dir.substr(0, sl)) + "/rover_cam.py";
            }
            if (access(yoloModel.c_str(), R_OK) != 0 || access(camScript.c_str(), R_OK) != 0) {
                std::fprintf(stderr, "[cam] --yolo-model: cannot read model '%s' or script '%s'; using the normal camera command\n",
                             yoloModel.c_str(), camScript.c_str());
            } else {
                camCmd = "exec nice -n 10 python3 '" + camScript + "' --model '" + yoloModel + "' --width " + std::to_string(camW) +
                         " --height " + std::to_string(camH) + " --fps " + std::to_string(camFps) +
                         " --quality " + std::to_string(camQ) + " --rotate 180 --threshold " + std::to_string(yoloThr);
                if (!yoloLabels.empty()) camCmd += " --labels '" + yoloLabels + "'";
                if (useHq && g_multi && hqQ >= 1 && hqQ <= 100 && hqFps >= 1) {
                    writeModeFile(0);
                    camCmd += " --hq-width " + std::to_string(hqW) + " --hq-height " + std::to_string(hqH) +
                              " --hq-quality " + std::to_string(hqQ) + " --hq-fps " + std::to_string(hqFps) +
                              " --mode-file '" + std::string(kModeFile) + "'";
                    modeT = std::thread(modeFileThread);
                    std::fprintf(stderr, "[cam] picture: %dx%d q%d %dfps on the hotspot; %dx%d q%d %dfps on the home network\n",
                                 camW, camH, camQ, camFps, hqW, hqH, hqQ, hqFps);
                }
                g_camAlways = !camOnDemand;
                std::fprintf(stderr, "[cam] YOLO via Picamera2 (%s)%s\n", yoloModel.c_str(),
                             g_camAlways ? "; camera stays running. First start can take ~1 min." : "");
            }
        }
        cam = std::thread(camThread, camCmd);

        httplib::Server svr;
        svr.new_task_queue = [] { return new httplib::ThreadPool(8); };
        svr.set_tcp_nodelay(true);
        svr.set_socket_options([](socket_t sock) {
            httplib::default_socket_options(sock);
            int sz = 64 * 1024;
            setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*)&sz, sizeof sz);
        });

        svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
            if (g_multi) {
                if (onOwnHotspot(req.remote_addr)) { g_lastClient = secs(); return httplib::Server::HandlerResponse::Unhandled; }
                if (g_password.size() < 8) {
                    res.status = 403;
                    res.set_content("Remote access is disabled: no web password is set on the rover.", "text/plain");
                    return httplib::Server::HandlerResponse::Handled;
                }
                if (!passOk(req.get_header_value("Authorization"), req.remote_addr)) {
                    res.status = 401;
                    res.set_header("WWW-Authenticate", "Basic realm=\"Rover\", charset=\"UTF-8\"");
                    res.set_content("Password needed", "text/plain");
                    return httplib::Server::HandlerResponse::Handled;
                }
                g_lastClient = secs();
                return httplib::Server::HandlerResponse::Unhandled;
            }
            if (!clientAllowed(req.remote_addr)) {
                res.status = 403;
                return httplib::Server::HandlerResponse::Handled;
            }
            g_lastClient = secs();
            return httplib::Server::HandlerResponse::Unhandled;
        });

        svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(PAGE, "text/html; charset=utf-8");
        });
        svr.Get("/favicon.ico", [](const httplib::Request&, httplib::Response& res) { res.status = 204; });

        svr.Get("/c", [](const httplib::Request& req, httplib::Response& res) {
            const int k = req.has_param("k") ? std::atoi(req.get_param_value("k").c_str()) : 0;
            const double v = clampd(req.has_param("v") ? std::atoi(req.get_param_value("v").c_str()) : 50, 0, 100) / 100.0;
            auto bit = [&](int b) { return (k & b) ? 1 : 0; };
            {
                const double t = steadyS(), gap = t - g_lastCtlT.load();
                if (k != 0 && g_lastCtlK.load() != 0 && gap > 0.3 && gap < 3.0) g_ctlLagUntil = t + 20.0;
                g_lastCtlT = t; g_lastCtlK = k;
            }
            {
                std::lock_guard<std::mutex> lk(g.m);
                Input& w = g.web;
                w.thr = (bit(1) - bit(2)) * v;
                w.str_dir = bit(8) - bit(4);
                w.pan  = bit(128) - bit(64);
                w.tilt = bit(32) - bit(16);
                if (k & 256) w.ctr_steer = true;
                if (k & 512) w.ctr_cam = true;
                w.stamp = Clock::now();
            }
            res.status = 204;
        });

        svr.Get("/s", [](const httplib::Request&, httplib::Response& res) {
            g_lastPoll = secs();
            std::string j;
            { std::lock_guard<std::mutex> lk(g_statusM); j = g_status; }
            res.set_header("Cache-Control", "no-store");
            res.set_content(j, "application/json");
        });
        svr.Get("/e", [](const httplib::Request&, httplib::Response& res) {
            g_estop = true;
            res.status = 204;
        });
        svr.Get("/r", [](const httplib::Request&, httplib::Response& res) {
            g_estop = false;
            res.status = 204;
        });
        svr.Get("/x", [](const httplib::Request&, httplib::Response& res) {
            g_autoCancel = true;
            res.status = 204;
        });
        svr.Get("/a", [](const httplib::Request& req, httplib::Response& res) {
            g_lastPoll = secs();
            std::string err;
            if (g_estop) err = "E-STOP is on. Reset it first.";
            else if (!g_sonarOk) err = "Ultrasonic sensors are not running (see the terminal on the Pi).";
            else if (!req.has_param("lat") || !req.has_param("lon")) err = "Missing latitude/longitude.";
            if (err.empty()) {
                const double lat = std::atof(req.get_param_value("lat").c_str());
                const double lon = std::atof(req.get_param_value("lon").c_str());
                const bool hk = req.has_param("h");
                const double h = hk ? std::atof(req.get_param_value("h").c_str()) : 0.0;
                if (lat < -90 || lat > 90 || lon < -180 || lon > 180) err = "Latitude/longitude out of range.";
                else {
                    std::lock_guard<std::mutex> lk(g_engM);
                    err = g_eng.start(lat, lon, hk, h, secs());
                }
            }
            res.set_content(err.empty() ? "OK" : err, "text/plain");
        });

        svr.Get("/stream", [](const httplib::Request&, httplib::Response& res) {
            auto lastSeq = std::make_shared<uint64_t>(0);
            auto guard = std::make_shared<Viewer>();
            res.set_chunked_content_provider(
                "multipart/x-mixed-replace; boundary=f",
                [lastSeq, guard](size_t, httplib::DataSink& sink) {
                    std::shared_ptr<const std::string> j;
                    {
                        std::unique_lock<std::mutex> lk(g_frame.m);
                        if (!g_frame.cv.wait_for(lk, 1s, [&] { return g_frame.seq != *lastSeq || !g_run; }))
                            return true;
                        if (!g_run) return false;
                        *lastSeq = g_frame.seq;
                        j = g_frame.jpg;
                    }
                    if (!j) return true;
                    char hdr[96];
                    const int hn = std::snprintf(hdr, sizeof hdr,
                        "--f\r\nContent-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n", j->size());
                    if (!sink.write(hdr, (size_t)hn)) return false;
                    if (!sink.write(j->data(), j->size())) return false;
                    return sink.write("\r\n", 2);
                });
        });

        std::thread watcher([&] {
            while (g_run) napMs(100);
            g_frame.cv.notify_all();
            g_vcv.notify_all();
            svr.stop();
        });
        std::thread netT;
        if (g_multi) netT = std::thread(netThread, iface, homeConn, hotspot, homeRetry);
        if (svr.bind_to_port(g_multi ? std::string("0.0.0.0") : ipStr(g_net.ip), port)) {
            if (g_multi)
                std::fprintf(stderr, "[web] listening on every address, port %d: http://<rover address>:%d/ "
                                     "(addresses are printed in the [net] lines)\n", port, port);
            else printLinks(port);
            svr.listen_after_bind();
        } else {
            std::fprintf(stderr, "[web] could not listen on %s:%d (port already in use?)\n",
                         ipStr(g_net.ip).c_str(), port);
        }
        g_run = false;
        watcher.join();
        if (netT.joinable()) netT.join();
    } else {
        while (g_run) napMs(100);
    }

    g_run = false;
    g_vcv.notify_all();
    if (pad.joinable()) pad.join();
    if (cam.joinable()) cam.join();
    if (modeT.joinable()) modeT.join();
    if (autoT.joinable()) autoT.join();
    if (sonar.joinable()) sonar.join();
    if (gps.joinable()) gps.join();
    ctl.join();
    bus.end();
    return 0;
}
