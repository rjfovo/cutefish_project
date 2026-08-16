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
