// Qt6 wrapper for the Ajazz AKP03E (Revision 2), USB 0300:3002.
//
// Akp03Device exposes:
//   * initialize()  - find/open the device and prepare it
//   * sendImage()   - push a QImage onto one of the 6 LCD keys
//   * setBrightness(), clearKey(), clearAll()
//   * startListening()/stopListening() - run the input loop on a Qt thread
//   * typed signals for every button / encoder event
//
// The device is driven over the kernel hidraw interface (no hidapi/libusb).
// The blocking input loop runs in an internal QThread (Akp03Reader); its events
// are delivered to your slots in the receiver's thread via queued connections.

#pragma once

#include <QObject>
#include <QThread>
#include <QImage>
#include <QString>
#include <atomic>

// ---------------------------------------------------------------------------
// Internal worker: blocking hidraw read loop on its own thread.
// ---------------------------------------------------------------------------
class Akp03Reader : public QThread {
    Q_OBJECT
public:
    explicit Akp03Reader(int fd, QObject* parent = nullptr);

    // Ask the loop to finish; the thread exits within one poll interval.
    void requestStop();

signals:
    void buttonPressed(int key);
    void buttonReleased(int key);
    void encoderPressed(int encoder);
    void encoderReleased(int encoder);
    void encoderTurned(int encoder, int delta); // delta: +1 = CW, -1 = CCW
    void deviceDisconnected();
    void errorOccurred(const QString& message);

protected:
    void run() override;

private:
    int m_fd;
    std::atomic<bool> m_stop{false};
};

// ---------------------------------------------------------------------------
// Public device class.
// ---------------------------------------------------------------------------
class Akp03Device : public QObject {
    Q_OBJECT
public:
    // Logical layout of the device.
    static constexpr int KeyCount = 9;      // 6 LCD keys (0..5) + 3 plain buttons (6..8)
    static constexpr int LcdKeyCount = 6;   // only keys 0..5 have screens
    static constexpr int EncoderCount = 3;  // 0 = left, 1 = middle, 2 = right

    // Encoder identifiers, for readability when connecting to encoder signals.
    enum class Encoder { Left = 0, Middle = 1, Right = 2 };
    Q_ENUM(Encoder)

    explicit Akp03Device(QObject* parent = nullptr);
    ~Akp03Device() override;

    // Find, open and prepare the device (init handshake, brightness, clear).
    // Returns false and (optionally) fills *errorOut on failure.
    bool initialize(QString* errorOut = nullptr);

    bool isOpen() const { return m_fd >= 0; }

    // Push an image onto an LCD key (0..5). The image is scaled to 60x60,
    // rotated to the device's orientation and JPEG-encoded internally.
    bool sendImage(int key, const QImage& image);

    bool setBrightness(int percent);   // 0..100
    bool clearKey(int key);            // blank one key
    bool clearAll();                   // blank every key

    // Start/stop the background input thread. Safe to call repeatedly.
    void startListening();
    void stopListening();

    // Stop listening, sleep the device and close the handle.
    void close();

signals:
    void buttonPressed(int key);
    void buttonReleased(int key);
    void encoderPressed(int encoder);
    void encoderReleased(int encoder);
    void encoderTurned(int encoder, int delta); // delta: +1 = CW, -1 = CCW
    void deviceDisconnected();
    void errorOccurred(const QString& message);

private:
    bool writeExtended(QByteArray cmd);                 // pad + write one report
    bool sendImagePayload(int key, const QByteArray& jpeg);

    int m_fd = -1;
    Akp03Reader* m_reader = nullptr;
};
