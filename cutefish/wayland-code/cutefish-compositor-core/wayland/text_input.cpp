#include "wayland/text_input.h"
#include "wayland/server.h"
#include "wm/window.h"

#include "text-input-unstable-v3-server-protocol.h"
#include "wm/workspace.h"

#include <QDebug>
#include <QString>

#include <algorithm>

namespace Cutefish {

namespace {

constexpr uint32_t kTextInputVersion = 1;

struct TextInputData {
    TextInputManager *manager = nullptr;
    bool enabled = false;
};

void textInputDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void textInputEnable(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    auto *data = static_cast<TextInputData *>(wl_resource_get_user_data(resource));
    if (!data || data->enabled)
        return;
    data->enabled = true;
    if (Window *window = data->manager->server()->workspace()->activeWindow())
        zwp_text_input_v3_send_enter(resource, window->surface());
    zwp_text_input_v3_send_done(resource, wl_display_next_serial(wl_client_get_display(client)));
}

void textInputDisable(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    auto *data = static_cast<TextInputData *>(wl_resource_get_user_data(resource));
    if (!data || !data->enabled)
        return;
    data->enabled = false;
    if (Window *window = data->manager->server()->workspace()->activeWindow())
        zwp_text_input_v3_send_leave(resource, window->surface());
    zwp_text_input_v3_send_done(resource, wl_display_next_serial(wl_client_get_display(client)));
}

void textInputSetSurroundingText(wl_client *client, wl_resource *resource,
                                 const char *text, int32_t cursor, int32_t anchor)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(text)
    Q_UNUSED(cursor)
    Q_UNUSED(anchor)
}

void textInputSetTextChangeCause(wl_client *client, wl_resource *resource, uint32_t cause)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(cause)
}

void textInputSetContentType(wl_client *client, wl_resource *resource, uint32_t hint, uint32_t purpose)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(hint)
    Q_UNUSED(purpose)
}

void textInputSetCursorRectangle(wl_client *client, wl_resource *resource,
                                 int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

void textInputCommit(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    zwp_text_input_v3_send_done(resource, wl_display_next_serial(wl_client_get_display(client)));
}

struct zwp_text_input_v3_interface textInputImplementation;

void managerDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void managerGetTextInput(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat)
{
    Q_UNUSED(seat)
    auto *manager = static_cast<TextInputManager *>(wl_resource_get_user_data(resource));
    auto *data = new TextInputData;
    data->manager = manager;
    auto *textInput = wl_resource_create(client, &zwp_text_input_v3_interface,
                                         kTextInputVersion, id);
    if (!textInput) {
        delete data;
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(textInput, &textInputImplementation, data,
                                   [](wl_resource *r) { delete static_cast<TextInputData *>(wl_resource_get_user_data(r)); });
}

struct zwp_text_input_manager_v3_interface managerImplementation;

void installImplementations()
{
    static bool installed = false;
    if (installed)
        return;
    installed = true;

    textInputImplementation.destroy = textInputDestroy;
    textInputImplementation.enable = textInputEnable;
    textInputImplementation.disable = textInputDisable;
    textInputImplementation.set_surrounding_text = textInputSetSurroundingText;
    textInputImplementation.set_text_change_cause = textInputSetTextChangeCause;
    textInputImplementation.set_content_type = textInputSetContentType;
    textInputImplementation.set_cursor_rectangle = textInputSetCursorRectangle;
    textInputImplementation.commit = textInputCommit;

    managerImplementation.destroy = managerDestroy;
    managerImplementation.get_text_input = managerGetTextInput;
}

void managerBind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *manager = static_cast<TextInputManager *>(data);
    auto *resource = wl_resource_create(client, &zwp_text_input_manager_v3_interface,
                                        std::min(version, kTextInputVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &managerImplementation, manager, nullptr);
}

} // namespace

TextInputManager::TextInputManager(WaylandServer *server, QObject *parent)
    : QObject(parent)
    , m_server(server)
{
    installImplementations();
}

bool TextInputManager::registerDisplay(wl_display *display)
{
    return wl_global_create(display, &zwp_text_input_manager_v3_interface,
                            kTextInputVersion, this, managerBind) != nullptr;
}

WaylandServer *TextInputManager::server() const
{
    return m_server;
}

} // namespace Cutefish
