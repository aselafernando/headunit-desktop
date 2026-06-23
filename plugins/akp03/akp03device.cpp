#include "akp03device.h"

#include <QByteArray>
#include <QTransform>

#include <cstdint>
#include <cstring>
#include <cerrno>
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <glob.h>
#include <sys/ioctl.h>
#include <linux/hidraw.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third_party/stb_image_write.h"

// ---------------------------------------------------------------------------
// Protocol constants (Ajazz AKP03E Rev 2, mirajazz protocol v3)
// ---------------------------------------------------------------------------
namespace {

constexpr uint16_t VID = 0x0300;
constexpr uint16_t PID = 0x3002;

// Control interface advertises HID usage page 0xFF80 (encoded "06 80 FF").
constexpr uint8_t USAGE_PAGE_TAG[3] = {0x06, 0x80, 0xFF};

constexpr size_t PACKET_SIZE = 1024;            // protocol >= 2
constexpr size_t REPORT_LEN = 1 + PACKET_SIZE;  // leading report-id byte

constexpr int IMG_SIZE = 60;

// Build a "CRT\0\0" command with the given opcode/parameter tail.
QByteArray crt(std::initializer_list<uint8_t> tail) {
    QByteArray v;
    static const uint8_t head[6] = {0x00, 0x43, 0x52, 0x54, 0x00, 0x00}; // 0x00 + "CRT" + 0x00 0x00
    v.append(reinterpret_cast<const char*>(head), 6);
    for (uint8_t b : tail) v.append(static_cast<char>(b));
    return v;
}

// Scan a hidraw report descriptor for usage page 0xFF80.
bool hasControlUsagePage(int fd) {
    int descSize = 0;
    if (ioctl(fd, HIDIOCGRDESCSIZE, &descSize) < 0) return false;

    struct hidraw_report_descriptor desc;
    std::memset(&desc, 0, sizeof(desc));
    desc.size = descSize;
    if (ioctl(fd, HIDIOCGRDESC, &desc) < 0) return false;

    for (int i = 0; i + 2 < descSize; i++) {
        if (desc.value[i] == USAGE_PAGE_TAG[0] &&
            desc.value[i + 1] == USAGE_PAGE_TAG[1] &&
            desc.value[i + 2] == USAGE_PAGE_TAG[2]) {
            return true;
        }
    }
    return false;
}

// Scan /dev/hidraw* for our VID/PID, preferring the control interface node.
// Returns an open fd (and the path), or -1.
int openHidraw(QString* pathOut) {
    glob_t g;
    if (glob("/dev/hidraw*", 0, nullptr, &g) != 0) return -1;

    int fallbackFd = -1;
    QString fallbackPath;

    for (size_t i = 0; i < g.gl_pathc; i++) {
        const char* path = g.gl_pathv[i];
        int fd = open(path, O_RDWR | O_CLOEXEC);
        if (fd < 0) continue;

        struct hidraw_devinfo info;
        std::memset(&info, 0, sizeof(info));
        if (ioctl(fd, HIDIOCGRAWINFO, &info) < 0) {
            ::close(fd);
            continue;
        }
        if ((uint16_t)info.vendor != VID || (uint16_t)info.product != PID) {
            ::close(fd);
            continue;
        }

        if (hasControlUsagePage(fd)) {
            if (fallbackFd >= 0) ::close(fallbackFd);
            if (pathOut) *pathOut = QString::fromUtf8(path);
            globfree(&g);
            return fd;
        }
        if (fallbackFd < 0) {
            fallbackFd = fd;
            fallbackPath = QString::fromUtf8(path);
        } else {
            ::close(fd);
        }
    }

    globfree(&g);
    if (fallbackFd >= 0) {
        if (pathOut) *pathOut = fallbackPath;
        return fallbackFd;
    }
    return -1;
}

void jpegSink(void* ctx, void* data, int len) {
    auto* out = static_cast<QByteArray*>(ctx);
    out->append(static_cast<const char*>(data), len);
}

} // namespace

// ---------------------------------------------------------------------------
// Akp03Device
// ---------------------------------------------------------------------------

Akp03Device::Akp03Device(QObject* parent) : QObject(parent) {}

Akp03Device::~Akp03Device() {
    close();
}

bool Akp03Device::initialize(QString* errorOut) {
    if (m_fd >= 0) return true;

    QString path;
    int fd = openHidraw(&path);
    if (fd < 0) {
        if (errorOut)
            *errorOut = QStringLiteral(
                "No Ajazz AKP03E Rev 2 (0300:3002) found, or its hidraw node is not "
                "accessible. Install 40-opendeck-akp03.rules and replug, or run as root.");
        return false;
    }
    m_fd = fd;

    // Init handshake: DIS, then LIG 0 0 0 0.
    bool ok = writeExtended(crt({'D', 'I', 'S'})) &&
              writeExtended(crt({'L', 'I', 'G', 0x00, 0x00, 0x00, 0x00}));
    if (!ok) {
        if (errorOut) *errorOut = QStringLiteral("Device init handshake failed.");
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    setBrightness(80);
    clearAll();
    return true;
}

bool Akp03Device::writeExtended(QByteArray cmd) {
    if (m_fd < 0) return false;
    if (cmd.size() < static_cast<int>(REPORT_LEN))
        cmd.append(static_cast<int>(REPORT_LEN) - cmd.size(), '\0');
    ssize_t res = ::write(m_fd, cmd.constData(), cmd.size());
    return res >= 0;
}

bool Akp03Device::setBrightness(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    return writeExtended(crt({'L', 'I', 'G', 0x00, 0x00, (uint8_t)percent}));
}

bool Akp03Device::clearKey(int key) {
    if (key < 0 || key >= KeyCount) return false;
    return writeExtended(crt({'C', 'L', 'E', 0x00, 0x00, 0x00, (uint8_t)(key + 1)}));
}

bool Akp03Device::clearAll() {
    // CLE 0xFF clears all, then STP commits on protocol v2/v3.
    return writeExtended(crt({'C', 'L', 'E', 0x00, 0x00, 0x00, 0xFF})) &&
           writeExtended(crt({'S', 'T', 'P'}));
}

bool Akp03Device::sendImagePayload(int key, const QByteArray& jpeg) {
    uint16_t len = (uint16_t)jpeg.size();
    QByteArray header = crt({
        'B', 'A', 'T',
        0x00, 0x00,
        (uint8_t)(len >> 8), (uint8_t)(len & 0xFF),
        (uint8_t)(key + 1),
    });
    if (!writeExtended(header)) return false;

    // Image bytes in 1024-byte chunks, each its own 0x00-prefixed padded report.
    size_t remaining = jpeg.size();
    size_t page = 0;
    while (remaining > 0) {
        size_t thisLen = remaining < PACKET_SIZE ? remaining : PACKET_SIZE;
        size_t sent = page * PACKET_SIZE;

        QByteArray buf;
        buf.reserve(REPORT_LEN);
        buf.append('\0');
        buf.append(jpeg.constData() + sent, thisLen);
        buf.append(static_cast<int>(REPORT_LEN) - buf.size(), '\0');

        if (::write(m_fd, buf.constData(), buf.size()) < 0) return false;

        remaining -= thisLen;
        page++;
    }
    return true;
}

bool Akp03Device::sendImage(int key, const QImage& image) {
    if (m_fd < 0 || key < 0 || key >= LcdKeyCount || image.isNull()) return false;

    // Scale to 60x60, rotate 90 clockwise to match the device orientation,
    // then pack as tight RGB888.
    QImage img = image.scaled(IMG_SIZE, IMG_SIZE, Qt::IgnoreAspectRatio,
                              Qt::SmoothTransformation)
                     .transformed(QTransform().rotate(90)) // Qt: positive = clockwise
                     .convertToFormat(QImage::Format_RGB888);

    const int w = img.width();
    const int h = img.height();
    std::vector<uint8_t> rgb;
    rgb.reserve((size_t)w * h * 3);
    for (int y = 0; y < h; y++) {
        const uchar* line = img.constScanLine(y);
        rgb.insert(rgb.end(), line, line + (size_t)w * 3);
    }

    QByteArray jpeg;
    stbi_write_jpg_to_func(jpegSink, &jpeg, w, h, 3, rgb.data(), 90);

    return sendImagePayload(key, jpeg);
}

void Akp03Device::startListening() {
    if (m_fd < 0 || m_reader) return;

    m_reader = new Akp03Reader(m_fd);

    // Forward the worker's events to our own signals. The worker emits from its
    // thread, so these queued connections marshal them to the receiver thread.
    connect(m_reader, &Akp03Reader::buttonPressed, this, &Akp03Device::buttonPressed);
    connect(m_reader, &Akp03Reader::buttonReleased, this, &Akp03Device::buttonReleased);
    connect(m_reader, &Akp03Reader::encoderPressed, this, &Akp03Device::encoderPressed);
    connect(m_reader, &Akp03Reader::encoderReleased, this, &Akp03Device::encoderReleased);
    connect(m_reader, &Akp03Reader::encoderTurned, this, &Akp03Device::encoderTurned);
    connect(m_reader, &Akp03Reader::deviceDisconnected, this, &Akp03Device::deviceDisconnected);
    connect(m_reader, &Akp03Reader::errorOccurred, this, &Akp03Device::errorOccurred);

    m_reader->start();
}

void Akp03Device::stopListening() {
    if (!m_reader) return;
    m_reader->requestStop();
    m_reader->wait();
    delete m_reader;
    m_reader = nullptr;
}

void Akp03Device::close() {
    stopListening();
    if (m_fd >= 0) {
        // Sleep the device: CLE .. DC, then HAN.
        writeExtended(crt({'C', 'L', 'E', 0x00, 0x00, 'D', 'C'}));
        writeExtended(crt({'H', 'A', 'N'}));
        ::close(m_fd);
        m_fd = -1;
    }
}

// ---------------------------------------------------------------------------
// Akp03Reader (input thread)
// ---------------------------------------------------------------------------

Akp03Reader::Akp03Reader(int fd, QObject* parent) : QThread(parent), m_fd(fd) {}

void Akp03Reader::requestStop() {
    m_stop.store(true);
}

void Akp03Reader::run() {
    bool prevButtons[Akp03Device::KeyCount] = {false};
    bool prevEncoders[Akp03Device::EncoderCount] = {false};

    uint8_t buf[512];
    struct pollfd pfd;
    pfd.fd = m_fd;
    pfd.events = POLLIN;

    while (!m_stop.load()) {
        int pr = poll(&pfd, 1, 200); // 200ms so requestStop() is honored promptly
        if (pr < 0) {
            if (errno == EINTR) continue;
            emit errorOccurred(QStringLiteral("poll() failed: %1")
                                   .arg(QString::fromLocal8Bit(strerror(errno))));
            break;
        }
        if (pr == 0) continue;
        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            emit deviceDisconnected();
            break;
        }
        if (!(pfd.revents & POLLIN)) continue;

        ssize_t n = ::read(m_fd, buf, sizeof(buf));
        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;
            emit errorOccurred(QStringLiteral("read() failed: %1")
                                   .arg(QString::fromLocal8Bit(strerror(errno))));
            break;
        }

        // Protocol v3 packets are prefixed with ASCII "ACK".
        if (n < 11 || buf[0] != 'A' || buf[1] != 'C' || buf[2] != 'K') continue;

        const uint8_t input = buf[9];
        const uint8_t state = buf[10];

        // --- Buttons -------------------------------------------------------
        if (input <= 6 || input == 0x25 || input == 0x30 || input == 0x31) {
            bool cur[Akp03Device::KeyCount] = {false};
            if (input != 0) { // input == 0 is an "all released" snapshot
                int btn;
                if (input >= 1 && input <= 6) btn = input - 1; // LCD buttons 0..5
                else if (input == 0x25)       btn = 6;
                else if (input == 0x30)       btn = 7;
                else                          btn = 8; // 0x31
                cur[btn] = (state != 0);
            }
            for (int i = 0; i < Akp03Device::KeyCount; i++) {
                if (cur[i] != prevButtons[i]) {
                    if (cur[i]) emit buttonPressed(i);
                    else        emit buttonReleased(i);
                    prevButtons[i] = cur[i];
                }
            }
            continue;
        }

        // --- Encoder twist -------------------------------------------------
        if (input == 0x90 || input == 0x91 || input == 0x50 ||
            input == 0x51 || input == 0x60 || input == 0x61) {
            int encoder, delta;
            switch (input) {
                case 0x90: encoder = 0; delta = -1; break; // left CCW
                case 0x91: encoder = 0; delta = 1;  break; // left CW
                case 0x50: encoder = 1; delta = -1; break; // middle CCW
                case 0x51: encoder = 1; delta = 1;  break; // middle CW
                case 0x60: encoder = 2; delta = -1; break; // right CCW
                default:   encoder = 2; delta = 1;  break; // 0x61 right CW
            }
            emit encoderTurned(encoder, delta);
            continue;
        }

        // --- Encoder press -------------------------------------------------
        if (input >= 0x33 && input <= 0x35) {
            int enc = (input == 0x33) ? 0 : (input == 0x35) ? 1 : 2; // left, middle, right
            bool cur[Akp03Device::EncoderCount] = {false};
            cur[enc] = (state != 0);
            for (int i = 0; i < Akp03Device::EncoderCount; i++) {
                if (cur[i] != prevEncoders[i]) {
                    if (cur[i]) emit encoderPressed(i);
                    else        emit encoderReleased(i);
                    prevEncoders[i] = cur[i];
                }
            }
            continue;
        }
    }
}
