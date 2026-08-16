#pragma once

#include <QHash>
#include <QObject>
#include <QPointF>
#include <QString>

#include <wayland-server-core.h>

struct xkb_context;
struct xkb_keymap;
struct xkb_state;

namespace Cutefish {

// Seat owns wl_seat/wl_pointer/wl_keyboard/wl_touch resources for all client
// displays and translates compositor input into Wayland events. Stage-1 keeps
// the implementation minimal but protocol-correct for one focused client.
class Seat : public QObject {
    Q_OBJECT
public:
    explicit Seat(QObject *parent = nullptr);
    ~Seat() override;

    bool registerDisplay(wl_display *display);

    void setFocusSurface(wl_resource *surface);
    void clearFocus(wl_resource *surface);

    void pointerMotion(double dx, double dy);
    void pointerButton(uint32_t button, uint32_t state);
    void pointerAxis(uint32_t axis, double value, int32_t discrete);
    void pointerFrame();
    void touchDown(int id, double x, double y);
    void touchMotion(int id, double x, double y);
    void touchUp(int id);
    void touchFrame();
    void touchCancel();
    void keyboardKey(uint32_t evdevKey, uint32_t state);
    void setKeyboardLayout(const QString &layout);

    void pointerResourceCreated(wl_client *client);
    void keyboardResourceCreated(wl_client *client);

    QPointF pointerPosition() const;
    wl_resource *focusSurface() const;

    struct ClientState {
        QList<wl_resource *> pointers;
        QList<wl_resource *> keyboards;
        QList<wl_resource *> touches;
    };

    ClientState *stateForClient(wl_client *client, bool create);
    void removeResource(wl_resource *resource);

private:
    void sendPointerEnter(wl_client *client, wl_resource *surface);
    void sendPointerLeave(wl_client *client, wl_resource *surface);
    void sendKeyboardEnter(wl_client *client, wl_resource *surface);
    void sendKeyboardLeave(wl_client *client, wl_resource *surface);
    void sendModifiers(wl_resource *keyboard);
    void ensureKeymap();

    QHash<wl_client *, ClientState> m_clients;
    wl_resource *m_focusSurface = nullptr;
    wl_client *m_focusClient = nullptr;
    QPointF m_pointerPosition;
    uint32_t m_pointerButtonSerial = 0;

    xkb_context *m_xkbContext = nullptr;
    xkb_keymap *m_xkbKeymap = nullptr;
    xkb_state *m_xkbState = nullptr;
    int m_keymapFd = -1;
    size_t m_keymapSize = 0;
    QString m_layout;
};

} // namespace Cutefish
