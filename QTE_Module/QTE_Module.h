#pragma once
#include <string>

namespace NS_QTE_Module
{
class ISprite
{
public:
    virtual void DrawImage(const int x, const int y, const int transparency = 255) = 0;
    virtual void DrawImageRect(const int x, const int y, const int srcWidth, const int srcHeight, const int transparency = 255) = 0;
    virtual void DrawImageScaled(const int x, const int y, const int width, const int height, const int transparency = 255) = 0;
    virtual void Load(const std::wstring& filepath) = 0;
    virtual ISprite* Create() = 0;
    virtual ~ISprite() {};
    virtual void OnDeviceLost() = 0;
    virtual void OnDeviceReset() = 0;
};

class QTE_Module
{
public:

    enum class BarResult { None, Failure, Normal, Success };

    bool Update();
    void Render();
    void Finalize();

    void SetBars(ISprite* whiteBar, ISprite* blackBar, int screenWidth, int screenHeight);
    void SetCircleSprites(ISprite* growingCircle, ISprite* targetCircle, ISprite* button, int screenWidth, int screenHeight);
    void StartBarAnimation();
    void StartCircleAnimation();
    void StopBarAnimation();
    void StopCircleAnimation();
    BarResult GetBarResult() const;
    float GetTimingCloseness() const;

private:

    ISprite* m_sprGrowingCircle = nullptr;
    ISprite* m_sprTargetCircle = nullptr;
    ISprite* m_sprButton = nullptr;
    int m_screenWidth = 0;
    int m_screenHeight = 0;

    unsigned long long m_circleAnimStartTime = 0;
    bool m_circleAnimActive = false;
    int m_circleAnimSize = 0;
    BarResult m_circleResult = BarResult::None;
    unsigned long long m_circleStopWaitStart = 0;
    float m_circleStopCloseness = 0.0f;

    static const int TARGET_CIRCLE_SIZE = 176;
    static const int START_CIRCLE_SIZE = 48;
    static const int MAX_CIRCLE_SIZE = 304;
    static const int BUTTON_SIZE = 82;
    static const int CIRCLE_MATCH_MS = 2000;
    static const int CIRCLE_OVERSHOOT_MS = 2000;
    static const int CIRCLE_STOP_WAIT_MS = 500;
    static const int FRAME_MS = 17;
    static const int SUCCESS_WINDOW_MS = 2 * FRAME_MS;
    static const int NORMAL_WINDOW_MS = 10 * FRAME_MS;
};
}
