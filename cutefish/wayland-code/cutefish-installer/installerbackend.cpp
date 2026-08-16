#include "installerbackend.h"

#include <QDebug>

InstallerBackend::InstallerBackend(QObject *parent)
    : QObject(parent)
{
}

int InstallerBackend::currentStep() const
{
    return m_currentStep;
}

void InstallerBackend::setCurrentStep(int step)
{
    step = qBound(static_cast<int>(WelcomeStep), step, static_cast<int>(FinishStep));
    if (step == m_currentStep)
        return;
    m_currentStep = step;
    emit currentStepChanged();
}

bool InstallerBackend::dangerousJobsAllowed() const
{
    // Stage-0 safety guard: the installer skeleton can never execute disk,
    // partition, bootloader or efivarfs jobs. Disk jobs are implemented only
    // after dedicated-hardware validation.
    return false;
}

QString InstallerBackend::lastError() const
{
    return m_lastError;
}

void InstallerBackend::setLastError(const QString &error)
{
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}

void InstallerBackend::next()
{
    if (m_currentStep < FinishStep)
        setCurrentStep(m_currentStep + 1);
}

void InstallerBackend::back()
{
    if (m_currentStep > WelcomeStep)
        setCurrentStep(m_currentStep - 1);
}

bool InstallerBackend::beginInstall()
{
    setLastError(QStringLiteral("Installation jobs are disabled in this build. "
                                "They require dedicated hardware validation and must never run on the shared test server."));
    qWarning().noquote() << m_lastError;
    return false;
}
