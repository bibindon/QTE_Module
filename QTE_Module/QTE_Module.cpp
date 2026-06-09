#include "QTE_Module.h"
#include <Windows.h>

using namespace NS_QTE_Module;

bool QTE_Module::Update()
{
    if (!m_barAnimActive && m_barStopWaitStart > 0)
    {
        if (GetTickCount64() - m_barStopWaitStart >= BAR_STOP_WAIT_MS)
        {
            return true;
        }
    }
    return false;
}

void QTE_Module::Render()
{
    if (m_sprBlackBar != nullptr)
    {
        m_sprBlackBar->DrawImage((m_screenWidth - BAR_WIDTH) / 2, (m_screenHeight - 32) / 2);
    }

    if (m_barAnimActive)
    {
        unsigned long long elapsed = GetTickCount64() - m_barAnimStartTime;

        if (elapsed < BAR_ANIM_GROW_MS)
        {
            m_barAnimWidth = (int)((double)elapsed / BAR_ANIM_GROW_MS * BAR_WIDTH);
        }
        else if (elapsed < BAR_ANIM_GROW_MS + BAR_ANIM_SHRINK_MS)
        {
            unsigned long long shrinkElapsed = elapsed - BAR_ANIM_GROW_MS;
            m_barAnimWidth = BAR_WIDTH - (int)((double)shrinkElapsed / BAR_ANIM_SHRINK_MS * BAR_WIDTH);
        }
        else
        {
            m_barAnimActive = false;
            m_barAnimWidth = 0;
            if (m_barResult == BarResult::None)
            {
                m_barResult = BarResult::Failure;
            }
            m_barStopWaitStart = GetTickCount64();
        }
    }

    if (m_sprWhiteBar != nullptr && m_barAnimWidth > 0)
    {
        const int barX = (m_screenWidth - BAR_WIDTH) / 2;
        const int barY = (m_screenHeight - 32) / 2;
        m_sprWhiteBar->DrawImageRect(barX, barY, m_barAnimWidth, 32);
    }
}

void QTE_Module::Finalize()
{
    if (m_sprWhiteBar != nullptr)
    {
        delete m_sprWhiteBar;
        m_sprWhiteBar = nullptr;
    }
    if (m_sprBlackBar != nullptr)
    {
        delete m_sprBlackBar;
        m_sprBlackBar = nullptr;
    }
}

void NS_QTE_Module::QTE_Module::SetBars(ISprite* whiteBar, ISprite* blackBar, int screenWidth, int screenHeight)
{
    m_sprWhiteBar = whiteBar;
    m_sprBlackBar = blackBar;
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    StartBarAnimation();
}

void NS_QTE_Module::QTE_Module::StartBarAnimation()
{
    m_barAnimStartTime = GetTickCount64();
    m_barAnimActive = true;
    m_barAnimWidth = 0;
    m_barResult = BarResult::None;
    m_barStopWaitStart = 0;
}

void NS_QTE_Module::QTE_Module::StopBarAnimation()
{
    if (!m_barAnimActive)
    {
        return;
    }

    unsigned long long elapsed = GetTickCount64() - m_barAnimStartTime;

    long long diff = (long long)elapsed - BAR_ANIM_GROW_MS;
    if (diff < 0)
    {
        diff = -diff;
    }

    if (diff <= SUCCESS_WINDOW_MS)
    {
        m_barResult = BarResult::Success;
    }
    else if (diff <= NORMAL_WINDOW_MS)
    {
        m_barResult = BarResult::Normal;
    }
    else
    {
        m_barResult = BarResult::Failure;
    }

    m_barAnimActive = false;
    m_barStopWaitStart = GetTickCount64();
}

QTE_Module::BarResult NS_QTE_Module::QTE_Module::GetBarResult() const
{
    return m_barResult;
}
