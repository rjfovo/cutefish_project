#include "wayland/seat.h"

#include <QDebug>

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <wayland-server-protocol.h>
#include <xkbcommon/xkbcommon.h>

namespace Cutefish {

namespace {

constexpr uint32_t kSeatVersion = 5;

struct ResourceData {
    Seat *seat = nullptr;
    wl_client *client = nullptr;
    wl_resource *resource = nullptr;
};

void destroyResourceData(wl_resource *resource)
{
    auto *data = static_cast<ResourceData *>(wl_resource_get_user_data(resource));
    if (data && data->seat)
        data->seat->removeResource(resource);
    delete data;
}

// wl_seat -------------------------------------------------------------
void seatGetPointer(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *seat = static_cast<ResourceData *>(wl_resource_get_user_data(resource))->seat;
    auto *rd = new ResourceData;
    rd->seat = seat;
    rd->client = client;
    rd->resource = wl_resource_create(client, &wl_pointer_interface,
                                      std::min<uint32_t>(kSeatVersion, wl_resource_get_version(resource)), id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    extern const struct wl_pointer_interface cutefishPointerImplementation;
    wl_resource_set_implementation(rd->resource, &cutefishPointerImplementation, rd, destroyResourceData);
    seat->stateForClient(client, true)->pointers.append(rd->resource);
    seat->pointerResourceCreated(client);
}

void seatGetKeyboard(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *seat = static_cast<ResourceData *>(wl_resource_get_user_data(resource))->seat;
    auto *rd = new ResourceData;
    rd->seat = seat;
    rd->client = client;
    rd->resource = wl_resource_create(client, &wl_keyboard_interface,
                                      std::min<uint32_t>(kSeatVersion, wl_resource_get_version(resource)), id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    extern const struct wl_keyboard_interface cutefishKeyboardImplementation;
    wl_resource_set_implementation(rd->resource, &cutefishKeyboardImplementation, rd, destroyResourceData);
    seat->stateForClient(client, true)->keyboards.append(rd->resource);
    seat->keyboardResourceCreated(client);
}

void seatGetTouch(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *seat = static_cast<ResourceData *>(wl_resource_get_user_data(resource))->seat;
    auto *rd = new ResourceData;
    rd->seat = seat;
    rd->client = client;
    rd->resource = wl_resource_create(client, &wl_touch_interface,
                                      std::min<uint32_t>(kSeatVersion, wl_resource_get_version(resource)), id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    extern const struct wl_touch_interface cutefishTouchImplementation;
    wl_resource_set_implementation(rd->resource, &cutefishTouchImplementation, rd, destroyResourceData);
    seat->stateForClient(client, true)->touches.append(rd->resource);
}

void seatRelease(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

struct wl_seat_interface seatImplementation;

// wl_pointer ----------------------------------------------------------
void pointerSetCursor(wl_client *client, wl_resource *resource, uint32_t serial,
                      wl_resource *surface, int32_t hotspotX, int32_t hotspotY)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(serial)
    Q_UNUSED(surface)
    Q_UNUSED(hotspotX)
    Q_UNUSED(hotspotY)
}

void pointerRelease(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct wl_pointer_interface cutefishPointerImplementation = {
    pointerSetCursor,
    pointerRelease,
};

// wl_keyboard ---------------------------------------------------------
void keyboardRelease(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct wl_keyboard_interface cutefishKeyboardImplementation = {
    keyboardRelease,
};

// wl_touch ------------------------------------------------------------
void touchRelease(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct wl_touch_interface cutefishTouchImplementation = {
    touchRelease,
};

void seatBind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *seat = static_cast<Seat *>(data);
    auto *rd = new ResourceData;
    rd->seat = seat;
    rd->client = client;
    rd->resource = wl_resource_create(client, &wl_seat_interface,
                                      std::min(version, kSeatVersion), id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(rd->resource, &seatImplementation, rd, destroyResourceData);
    wl_seat_send_capabilities(rd->resource, WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD);
    if (wl_resource_get_version(rd->resource) >= WL_SEAT_NAME_SINCE_VERSION)
        wl_seat_send_name(rd->resource, "cutefish-seat");
    seat->stateForClient(client, true);
}

} // namespace

Seat::Seat(QObject *parent)
    : QObject(parent)
{
    seatImplementation.get_pointer = seatGetPointer;
    seatImplementation.get_keyboard = seatGetKeyboard;
    seatImplementation.get_touch = seatGetTouch;
    seatImplementation.release = seatRelease;

    m_xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!m_xkbContext)
        return;
    m_xkbKeymap = xkb_keymap_new_from_names(m_xkbContext, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!m_xkbKeymap)
        return;
    m_xkbState = xkb_state_new(m_xkbKeymap);
    ensureKeymap();
}

Seat::~Seat()
{
    if (m_keymapFd >= 0)
        ::close(m_keymapFd);
    if (m_xkbState)
        xkb_state_unref(m_xkbState);
    if (m_xkbKeymap)
        xkb_keymap_unref(m_xkbKeymap);
    if (m_xkbContext)
        xkb_context_unref(m_xkbContext);
}

void Seat::ensureKeymap()
{
    if (m_keymapFd >= 0 || !m_xkbKeymap)
        return;
    char *text = xkb_keymap_get_as_string(m_xkbKeymap, XKB_KEYMAP_FORMAT_TEXT_V1);
    if (!text)
        return;
    m_keymapSize = std::strlen(text) + 1;

#ifdef SYS_memfd_create
    m_keymapFd = static_cast<int>(::syscall(SYS_memfd_create, "cutefish-xkb-keymap", MFD_CLOEXEC));
#else
    m_keymapFd = -1;
#endif
    if (m_keymapFd >= 0 && ::ftruncate(m_keymapFd, static_cast<off_t>(m_keymapSize)) == 0) {
        void *map = ::mmap(nullptr, m_keymapSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_keymapFd, 0);
        if (map != MAP_FAILED) {
            std::memcpy(map, text, m_keymapSize);
            ::munmap(map, m_keymapSize);
        }
        ::lseek(m_keymapFd, 0, SEEK_SET);
    } else {
        if (m_keymapFd >= 0) {
            ::close(m_keymapFd);
            m_keymapFd = -1;
        }
    }
    std::free(text);
}

bool Seat::registerDisplay(wl_display *display)
{
    return wl_global_create(display, &wl_seat_interface, kSeatVersion,
                            this, seatBind) != nullptr;
}

Seat::ClientState *Seat::stateForClient(wl_client *client, bool create)
{
    auto it = m_clients.find(client);
    if (it != m_clients.end())
        return &it.value();
    if (!create)
        return nullptr;
    return &m_clients[client];
}

void Seat::removeResource(wl_resource *resource)
{
    for (auto it = m_clients.begin(); it != m_clients.end();) {
        ClientState &state = it.value();
        state.pointers.removeAll(resource);
        state.keyboards.removeAll(resource);
        state.touches.removeAll(resource);
        if (state.pointers.isEmpty() && state.keyboards.isEmpty() && state.touches.isEmpty())
            it = m_clients.erase(it);
        else
            ++it;
    }
}

void Seat::setFocusSurface(wl_resource *surface)
{
    if (m_focusSurface == surface)
        return;

    wl_client *newClient = surface ? wl_resource_get_client(surface) : nullptr;

    if (m_focusSurface && m_focusClient) {
        sendPointerLeave(m_focusClient, m_focusSurface);
        sendKeyboardLeave(m_focusClient, m_focusSurface);
    }

    m_focusSurface = surface;
    m_focusClient = newClient;

    if (m_focusSurface && m_focusClient) {
        sendPointerEnter(m_focusClient, m_focusSurface);
        sendKeyboardEnter(m_focusClient, m_focusSurface);
    }
}

void Seat::clearFocus(wl_resource *surface)
{
    if (m_focusSurface == surface)
        setFocusSurface(nullptr);
}

void Seat::sendPointerEnter(wl_client *client, wl_resource *surface)
{
    ClientState *state = stateForClient(client, false);
    if (!state)
        return;
    const uint32_t serial = wl_display_next_serial(wl_client_get_display(client));
    for (wl_resource *pointer : state->pointers) {
        wl_pointer_send_enter(pointer, serial, surface,
                              wl_fixed_from_double(m_pointerPosition.x()),
                              wl_fixed_from_double(m_pointerPosition.y()));
    }
}

void Seat::sendPointerLeave(wl_client *client, wl_resource *surface)
{
    ClientState *state = stateForClient(client, false);
    if (!state)
        return;
    const uint32_t serial = wl_display_next_serial(wl_client_get_display(client));
    for (wl_resource *pointer : state->pointers)
        wl_pointer_send_leave(pointer, serial, surface);
}

void Seat::sendKeyboardEnter(wl_client *client, wl_resource *surface)
{
    ClientState *state = stateForClient(client, false);
    if (!state)
        return;
    const uint32_t serial = wl_display_next_serial(wl_client_get_display(client));
    for (wl_resource *keyboard : state->keyboards) {
        if (m_keymapFd >= 0) {
            wl_keyboard_send_keymap(keyboard, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1,
                                    m_keymapFd, static_cast<uint32_t>(m_keymapSize));
        }
        wl_array keys;
        wl_array_init(&keys);
        wl_keyboard_send_enter(keyboard, serial, surface, &keys);
        wl_array_release(&keys);
        sendModifiers(keyboard);
    }
}

void Seat::sendKeyboardLeave(wl_client *client, wl_resource *surface)
{
    ClientState *state = stateForClient(client, false);
    if (!state)
        return;
    const uint32_t serial = wl_display_next_serial(wl_client_get_display(client));
    for (wl_resource *keyboard : state->keyboards)
        wl_keyboard_send_leave(keyboard, serial, surface);
}

void Seat::sendModifiers(wl_resource *keyboard)
{
    if (!m_xkbState)
        return;
    xkb_mod_mask_t depressed = 0;
    xkb_mod_mask_t latched = 0;
    xkb_mod_mask_t locked = 0;
    const uint32_t serial = wl_display_next_serial(
        wl_client_get_display(wl_resource_get_client(keyboard)));
    xkb_mod_mask_t group = xkb_state_serialize_mods(m_xkbState, XKB_STATE_MODS_DEPRESSED);
    depressed = group;
    group = xkb_state_serialize_mods(m_xkbState, XKB_STATE_MODS_LATCHED);
    latched = group;
    group = xkb_state_serialize_mods(m_xkbState, XKB_STATE_MODS_LOCKED);
    locked = group;
    group = xkb_state_serialize_layout(m_xkbState, XKB_STATE_LAYOUT_EFFECTIVE);
    wl_keyboard_send_modifiers(keyboard, serial, depressed, latched, locked, group);
}

void Seat::pointerResourceCreated(wl_client *client)
{
    if (m_focusClient == client && m_focusSurface)
        sendPointerEnter(client, m_focusSurface);
}

void Seat::keyboardResourceCreated(wl_client *client)
{
    if (m_focusClient == client && m_focusSurface) {
        ensureKeymap();
        sendKeyboardEnter(client, m_focusSurface);
    }
}

void Seat::pointerMotion(double dx, double dy)
{
    m_pointerPosition += QPointF(dx, dy);
    if (!m_focusClient)
        return;
    ClientState *state = stateForClient(m_focusClient, false);
    if (!state)
        return;
    for (wl_resource *pointer : state->pointers) {
        wl_pointer_send_motion(pointer, 0,
                               wl_fixed_from_double(m_pointerPosition.x()),
                               wl_fixed_from_double(m_pointerPosition.y()));
    }
}

void Seat::pointerButton(uint32_t button, uint32_t buttonState)
{
    if (!m_focusClient)
        return;
    ClientState *state = stateForClient(m_focusClient, false);
    if (!state)
        return;
    const uint32_t serial = wl_display_next_serial(wl_client_get_display(m_focusClient));
    for (wl_resource *pointer : state->pointers)
        wl_pointer_send_button(pointer, serial, 0, button, buttonState);
}

void Seat::keyboardKey(uint32_t evdevKey, uint32_t keyState)
{
    if (!m_focusClient)
        return;
    ClientState *state = stateForClient(m_focusClient, false);
    if (!state)
        return;
    const uint32_t keycode = evdevKey + 8;
    if (m_xkbState)
        xkb_state_update_key(m_xkbState, keycode,
                             keyState == WL_KEYBOARD_KEY_STATE_PRESSED
                                 ? XKB_KEY_DOWN : XKB_KEY_UP);

    const uint32_t serial = wl_display_next_serial(wl_client_get_display(m_focusClient));
    for (wl_resource *keyboard : state->keyboards) {
        wl_keyboard_send_key(keyboard, serial, 0, keycode, keyState);
        sendModifiers(keyboard);
    }
}

void Seat::setKeyboardLayout(const QString &layout)
{
    m_layout = layout;
    qInfo() << "Seat keyboard layout" << layout;
}

QPointF Seat::pointerPosition() const
{
    return m_pointerPosition;
}

wl_resource *Seat::focusSurface() const
{
    return m_focusSurface;
}

} // namespace Cutefish
