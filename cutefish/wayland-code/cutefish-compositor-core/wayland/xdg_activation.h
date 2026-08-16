/*
 * XdgActivation：xdg_activation_v1 token 与激活实现。
 * 职责：生成/校验 token，激活目标 Window 并发送 activated configure。
 * 安全边界：token 一次性使用；未知 token 忽略。
 */
#pragma once


#include <QHash>
#include <QObject>
#include <QString>

#include <wayland-server-core.h>

namespace Cutefish {

class WaylandServer;
struct ActivationToken;

class XdgActivation : public QObject {
    Q_OBJECT
public:
    explicit XdgActivation(WaylandServer *server, QObject *parent = nullptr);

    bool registerDisplay(wl_display *display);
    WaylandServer *server() const;

    QString createToken(ActivationToken *token);
    ActivationToken *takeToken(const QString &token);

private:
    WaylandServer *m_server = nullptr;
    QHash<QString, ActivationToken *> m_tokens;
    uint32_t m_nextTokenId = 1;
};

struct ActivationToken {
    QString value;
    QString appId;
    wl_resource *surface = nullptr;
    bool committed = false;
    bool used = false;
};

} // namespace Cutefish
