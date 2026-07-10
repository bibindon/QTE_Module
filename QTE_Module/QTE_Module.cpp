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
    const unsigned long long currentTime = GetTickCount64();

    if (m_sprTargetCircle != nullptr)
    {
        const int targetX = centerX - TARGET_CIRCLE_SIZE / 2;
        const int targetY = centerY - TARGET_CIRCLE_SIZE / 2;
        m_sprTargetCircle->DrawImageScaled(targetX, targetY, TARGET_CIRCLE_SIZE, TARGET_CIRCLE_SIZE);
    }

    if (m_circleAnimActive)
    {
        unsigned long long elapsed = currentTime - m_circleAnimStartTime;
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
            m_resultEffectStartTime = currentTime;
            m_circleStopWaitStart = currentTime;
        }
    }

    if (m_sprGrowingCircle != nullptr && m_circleAnimSize > 0)
    {
        const int circleX = centerX - m_circleAnimSize / 2;
        const int circleY = centerY - m_circleAnimSize / 2;
        m_sprGrowingCircle->DrawImageScaled(circleX, circleY, m_circleAnimSize, m_circleAnimSize, 210);
    }

    unsigned long long resultEffectElapsed = 0;
    bool resultEffectActive = false;
    if (m_circleResult != BarResult::None && m_resultEffectStartTime > 0)
    {
        resultEffectElapsed = currentTime - m_resultEffectStartTime;
        resultEffectActive = true;
    }

    bool successEffectActive = false;
    if (resultEffectActive && m_circleResult == BarResult::Success)
    {
        successEffectActive = true;
    }
    const unsigned long long successElapsed = resultEffectElapsed;

    if (successEffectActive && successElapsed < SUCCESS_BURST_MS && m_sprSuccessBurst != nullptr)
    {
        const int burstStartSize = 140;
        const int burstEndSize = 360;
        const int burstRange = burstEndSize - burstStartSize;
        const int burstSize = burstStartSize + (int)(successElapsed * burstRange / SUCCESS_BURST_MS);
        const int burstTransparency = 255 - (int)(successElapsed * 255 / SUCCESS_BURST_MS);
        const int burstX = centerX - burstSize / 2;
        const int burstY = centerY - burstSize / 2;
        m_sprSuccessBurst->DrawImageScaled(burstX, burstY, burstSize, burstSize, burstTransparency);
    }

    if (successEffectActive && successElapsed >= SUCCESS_WAVE_DELAY_MS && m_sprSuccessWave != nullptr)
    {
        const unsigned long long waveElapsed = successElapsed - SUCCESS_WAVE_DELAY_MS;
        if (waveElapsed < SUCCESS_WAVE_MS)
        {
            const int waveStartSize = 170;
            const int waveEndSize = 430;
            const int waveRange = waveEndSize - waveStartSize;
            const int waveSize = waveStartSize + (int)(waveElapsed * waveRange / SUCCESS_WAVE_MS);
            const int waveTransparency = 230 - (int)(waveElapsed * 230 / SUCCESS_WAVE_MS);
            const int waveX = centerX - waveSize / 2;
            const int waveY = centerY - waveSize / 2;
            m_sprSuccessWave->DrawImageScaled(waveX, waveY, waveSize, waveSize, waveTransparency);
        }
    }

    if (resultEffectActive && m_circleResult == BarResult::Normal && m_sprNormalWave != nullptr)
    {
        if (resultEffectElapsed < NORMAL_EFFECT_MS)
        {
            const int normalStartSize = 160;
            const int normalEndSize = 300;
            const int normalRange = normalEndSize - normalStartSize;
            const int normalSize = normalStartSize + (int)(resultEffectElapsed * normalRange / NORMAL_EFFECT_MS);
            const int normalTransparency = 210 - (int)(resultEffectElapsed * 210 / NORMAL_EFFECT_MS);
            const int normalX = centerX - normalSize / 2;
            const int normalY = centerY - normalSize / 2;
            m_sprNormalWave->DrawImageScaled(normalX, normalY, normalSize, normalSize, normalTransparency);
        }
    }

    if (resultEffectActive && m_circleResult == BarResult::Failure && m_sprFailureImpact != nullptr)
    {
        if (resultEffectElapsed < FAILURE_EFFECT_MS)
        {
            const int failureStartSize = 170;
            const int failureEndSize = 330;
            const int failureRange = failureEndSize - failureStartSize;
            const int failureSize = failureStartSize + (int)(resultEffectElapsed * failureRange / FAILURE_EFFECT_MS);
            const int failureTransparency = 235 - (int)(resultEffectElapsed * 235 / FAILURE_EFFECT_MS);
            const int failureX = centerX - failureSize / 2;
            const int failureY = centerY - failureSize / 2;
            m_sprFailureImpact->DrawImageScaled(failureX, failureY, failureSize, failureSize, failureTransparency);
        }
    }

    int buttonSize = BUTTON_SIZE;
    if (successEffectActive && successElapsed < SUCCESS_BUTTON_PULSE_MS)
    {
        const int pulsePeakMs = 110;
        const int pulseSize = 14;
        if (successElapsed < (unsigned long long)pulsePeakMs)
        {
            buttonSize += (int)(successElapsed * pulseSize / pulsePeakMs);
        }
        else
        {
            const unsigned long long pulseReturnElapsed = successElapsed - pulsePeakMs;
            const int pulseReturnMs = SUCCESS_BUTTON_PULSE_MS - pulsePeakMs;
            buttonSize += pulseSize - (int)(pulseReturnElapsed * pulseSize / pulseReturnMs);
        }
    }

    if (m_sprButton != nullptr)
    {
        const int buttonX = centerX - buttonSize / 2;
        const int buttonY = centerY - buttonSize / 2;
        m_sprButton->DrawImageScaled(buttonX, buttonY, buttonSize, buttonSize);
    }

    if (successEffectActive && successElapsed >= SUCCESS_SPARKLES_DELAY_MS && m_sprSuccessSparkles != nullptr)
    {
        const unsigned long long sparklesElapsed = successElapsed - SUCCESS_SPARKLES_DELAY_MS;
        if (sparklesElapsed < SUCCESS_SPARKLES_MS)
        {
            const int sparklesStartSize = 190;
            const int sparklesEndSize = 400;
            const int sparklesRange = sparklesEndSize - sparklesStartSize;
            const int sparklesSize = sparklesStartSize + (int)(sparklesElapsed * sparklesRange / SUCCESS_SPARKLES_MS);
            const int sparklesTransparency = 255 - (int)(sparklesElapsed * 255 / SUCCESS_SPARKLES_MS);
            const int sparklesX = centerX - sparklesSize / 2;
            const int sparklesY = centerY - sparklesSize / 2;
            m_sprSuccessSparkles->DrawImageScaled(sparklesX, sparklesY, sparklesSize, sparklesSize, sparklesTransparency);
        }
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
    if (m_sprSuccessBurst != nullptr)
    {
        delete m_sprSuccessBurst;
        m_sprSuccessBurst = nullptr;
    }
    if (m_sprSuccessWave != nullptr)
    {
        delete m_sprSuccessWave;
        m_sprSuccessWave = nullptr;
    }
    if (m_sprSuccessSparkles != nullptr)
    {
        delete m_sprSuccessSparkles;
        m_sprSuccessSparkles = nullptr;
    }
    if (m_sprNormalWave != nullptr)
    {
        delete m_sprNormalWave;
        m_sprNormalWave = nullptr;
    }
    if (m_sprFailureImpact != nullptr)
    {
        delete m_sprFailureImpact;
        m_sprFailureImpact = nullptr;
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

void NS_QTE_Module::QTE_Module::SetSuccessEffectSprites(ISprite* burst, ISprite* wave, ISprite* sparkles)
{
    m_sprSuccessBurst = burst;
    m_sprSuccessWave = wave;
    m_sprSuccessSparkles = sparkles;
}

void NS_QTE_Module::QTE_Module::SetResultEffectSprites(ISprite* normalWave, ISprite* failureImpact)
{
    m_sprNormalWave = normalWave;
    m_sprFailureImpact = failureImpact;
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
    m_resultEffectStartTime = 0;
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

    const unsigned long long currentTime = GetTickCount64();
    unsigned long long elapsed = currentTime - m_circleAnimStartTime;

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
    m_resultEffectStartTime = currentTime;
    m_circleStopWaitStart = currentTime;
}

QTE_Module::BarResult NS_QTE_Module::QTE_Module::GetBarResult() const
{
    return m_circleResult;
}
