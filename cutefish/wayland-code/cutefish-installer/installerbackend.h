/*
 * InstallerBackend：cutefish-installer 固定流程状态机。
 * 安全边界：stage-0 dangerousJobsAllowed=false，beginInstall 一律拒绝；
 * 分区/格式化/grub/efivarfs 只能由后续 polkit helper 在专用硬件环境执行。
 */
#pragma once

#include <QObject>
#include <QString>

class InstallerBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(int currentStep READ currentStep WRITE setCurrentStep NOTIFY currentStepChanged)
    Q_PROPERTY(bool dangerousJobsAllowed READ dangerousJobsAllowed CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
public:
    explicit InstallerBackend(QObject *parent = nullptr);

    enum Step {
        WelcomeStep = 0,
        DiskStep,
        PartitionStep,
        TimezoneStep,
        UserStep,
        SummaryStep,
        ProgressStep,
        FinishStep,
    };
    Q_ENUM(Step)

    int currentStep() const;
    void setCurrentStep(int step);
    bool dangerousJobsAllowed() const;
    QString lastError() const;

    Q_INVOKABLE void next();
    Q_INVOKABLE void back();
    Q_INVOKABLE bool beginInstall();

signals:
    void currentStepChanged();
    void lastErrorChanged();

private:
    void setLastError(const QString &error);

    int m_currentStep = WelcomeStep;
    QString m_lastError;
};
