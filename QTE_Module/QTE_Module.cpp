#include "QTE_Module.h"
#include <Windows.h>

using namespace NS_QTE_Module;

bool QTE_Module::Update()
{
    if (!m_circleAnimActive && m_circleStopWaitStart > 0)
    {
        if (GetTickCount64() - m_circleStopWaitStart >= CIRCLE_STOP_WAIT_MS)
        {
            return true;
        }
    }
    return false;
}

void QTE_Module::Render()
{
    const int centerX = m_screenWidth / 2;
    const int centerY = m_screenHeight / 2;

    if (m_sprTargetCircle != nullptr)
    {
        const int targetX = centerX - TARGET_CIRCLE_SIZE / 2;
        const int targetY = centerY - TARGET_CIRCLE_SIZE / 2;
        m_sprTargetCircle->DrawImageScaled(targetX, targetY, TARGET_CIRCLE_SIZE, TARGET_CIRCLE_SIZE);
    }

    if (m_circleAnimActive)
    {
        unsigned long long elapsed = GetTickCount64() - m_circleAnimStartTime;
        const int totalAnimMs = CIRCLE_MATCH_MS + CIRCLE_OVERSHOOT_MS;

        if (elapsed < (unsigned long long)totalAnimMs)
        {
            const int circleRange = MAX_CIRCLE_SIZE - START_CIRCLE_SIZE;
            m_circleAnimSize = START_CIRCLE_SIZE + (int)((double)elapsed / totalAnimMs * circleRange);
        }
        else
        {
            m_circleAnimActive = false;
            m_circleAnimSize = MAX_CIRCLE_SIZE;
            if (m_circleResult == BarResult::None)
            {
                m_circleResult = BarResult::Failure;
            }
            m_circleStopWaitStart = GetTickCount64();
        }
    }

    if (m_sprGrowingCircle != nullptr && m_circleAnimSize > 0)
    {
        const int circleX = centerX - m_circleAnimSize / 2;
        const int circleY = centerY - m_circleAnimSize / 2;
        m_sprGrowingCircle->DrawImageScaled(circleX, circleY, m_circleAnimSize, m_circleAnimSize, 210);
    }

    if (m_sprButton != nullptr)
    {
        const int buttonX = centerX - BUTTON_SIZE / 2;
        const int buttonY = centerY - BUTTON_SIZE / 2;
        m_sprButton->DrawImageScaled(buttonX, buttonY, BUTTON_SIZE, BUTTON_SIZE);
    }
}

void QTE_Module::Finalize()
{
    if (m_sprGrowingCircle != nullptr)
    {
        delete m_sprGrowingCircle;
        m_sprGrowingCircle = nullptr;
    }
    if (m_sprTargetCircle != nullptr)
    {
        delete m_sprTargetCircle;
        m_sprTargetCircle = nullptr;
    }
    if (m_sprButton != nullptr)
    {
        delete m_sprButton;
        m_sprButton = nullptr;
    }
}

void NS_QTE_Module::QTE_Module::SetBars(ISprite* whiteBar, ISprite* blackBar, int screenWidth, int screenHeight)
{
    SetCircleSprites(whiteBar, blackBar, nullptr, screenWidth, screenHeight);
}

void NS_QTE_Module::QTE_Module::SetCircleSprites(ISprite* growingCircle, ISprite* targetCircle, ISprite* button, int screenWidth, int screenHeight)
{
    m_sprGrowingCircle = growingCircle;
    m_sprTargetCircle = targetCircle;
    m_sprButton = button;
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    StartCircleAnimation();
}

void NS_QTE_Module::QTE_Module::StartBarAnimation()
{
    StartCircleAnimation();
}

void NS_QTE_Module::QTE_Module::StartCircleAnimation()
{
    m_circleAnimStartTime = GetTickCount64();
    m_circleAnimActive = true;
    m_circleAnimSize = START_CIRCLE_SIZE;
    m_circleResult = BarResult::None;
    m_circleStopWaitStart = 0;
}

void NS_QTE_Module::QTE_Module::StopBarAnimation()
{
    StopCircleAnimation();
}

void NS_QTE_Module::QTE_Module::StopCircleAnimation()
{
    if (!m_circleAnimActive)
    {
        return;
    }

    unsigned long long elapsed = GetTickCount64() - m_circleAnimStartTime;

    long long diff = (long long)elapsed - CIRCLE_MATCH_MS;
    if (diff < 0)
    {
        diff = -diff;
    }

    if (diff <= SUCCESS_WINDOW_MS)
    {
        m_circleResult = BarResult::Success;
    }
    else if (diff <= NORMAL_WINDOW_MS)
    {
        m_circleResult = BarResult::Normal;
    }
    else
    {
        m_circleResult = BarResult::Failure;
    }

    m_circleAnimActive = false;
    m_circleStopWaitStart = GetTickCount64();
}

QTE_Module::BarResult NS_QTE_Module::QTE_Module::GetBarResult() const
{
    return m_circleResult;
}
