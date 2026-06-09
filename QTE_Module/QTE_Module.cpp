#include "QTE_Module.h"
#include <sstream>
#include "HeaderOnlyCsv.hpp"
#include "CaesarCipher.h"

using namespace NS_QTE_Module;

bool QTE_Module::m_fastMode = false;

static std::vector<std::wstring> split(const std::wstring& s, wchar_t delim)
{
    std::vector<std::wstring> result;
    std::wstringstream ss(s);
    std::wstring item;

    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }

    return result;
}

void QTE_Module::Init(
    IFont* font,
    ISoundEffect* SE,
    ISprite* sprTextBack,
    ISprite* sprFade,
    const std::vector<Page>& pageList,
    const bool bEnglish)
{
    m_font = font;
    m_SE = SE;
    m_sprTextBack = sprTextBack;
    m_sprFade = sprFade;
    m_pageList = pageList;
    m_isFadeIn = true;

    if (m_font != nullptr)
    {
        m_font->Init(bEnglish);
    }
    if (m_SE != nullptr)
    {
        m_SE->Init();
    }

    InitConstValue();
}

void NS_QTE_Module::QTE_Module::Init(IFont* font,
                                        ISoundEffect* SE,
                                        ISprite* sprTextBack,
                                        ISprite* sprFade,
                                        const std::wstring& csvFile,
                                        ISprite* sprImage,
                                        const bool encrypt,
                                        const bool bEnglish)
{
    m_font = font;
    m_SE = SE;
    m_sprTextBack = sprTextBack;
    m_sprFade = sprFade;
    m_sprImage = sprImage;

    if (m_font != nullptr)
    {
        m_font->Init(bEnglish);
    }
    if (m_SE != nullptr)
    {
        m_SE->Init();
    }

    std::vector<std::vector<std::wstring> > vvs;
    if (encrypt == false)
    {
        vvs = csv::Read(csvFile);
    }
    else
    {
        auto workStr = CaesarCipher::DecryptFromFile(csvFile);
        vvs = csv::ReadFromString(workStr);
    }

    std::vector<Page> pageList;
    Page page;
    int pageNum = 0;
    std::vector<std::vector<std::wstring>> textList;
    for (size_t i = 1; i < vvs.size(); ++i)
    {
        std::vector<std::wstring> line = vvs.at(i);
        int pageNumTemp = std::stoi(line.at(0));

        // 新しいページ
        if (pageNum != pageNumTemp)
        {
            // 古いページを登録
            {
                // １行目だったら古いページはない
                if (page.GetSprite() == nullptr)
                {
                    // Do nothing
                }
                else
                {
                    page.SetTextList(textList);
                    pageList.push_back(page);

                    page.SetSprite(nullptr);
                    textList.clear();
                }
            }
            pageNum = pageNumTemp;
            std::wstring imagePath = line.at(1);

            ISprite* sprite = sprImage->Create();
            sprite->Load(imagePath);
            page.SetSprite(sprite);

            std::vector<std::wstring> texts = split(line.at(2), L'\n');

            for (size_t j = 0; j < texts.size(); ++j)
            {
                texts.at(j).erase(std::remove(texts.at(j).begin(), texts.at(j).end(), L'"'),
                                  texts.at(j).end());
            }
            textList.push_back(texts);
        }
        else
        {
            std::vector<std::wstring> texts = split(line.at(2), L'\n');
            for (size_t j = 0; j < texts.size(); ++j)
            {
                texts.at(j).erase(std::remove(texts.at(j).begin(), texts.at(j).end(), L'"'),
                                  texts.at(j).end());
            }
            textList.push_back(texts);
        }
    }
    page.SetTextList(textList);
    pageList.push_back(page);

    m_pageList = pageList;

    m_isFadeIn = true;

    InitConstValue();
}

void QTE_Module::Next()
{
    if (m_pageList.empty())
    {
        return;
    }
    if (m_waitNextCount < WAIT_NEXT_FRAME)
    {
        return;
    }
    int textIndex = m_pageList.at(m_pageIndex).GetTextIndex();
    int textIndexMax = (int)m_pageList.at(m_pageIndex).GetTextList().size();
    if (textIndex < textIndexMax - 1)
    {
        textIndex++;
    }
    else
    {
        m_FadeOutCount = 0;
        m_isFadeOut = true;
    }
    m_pageList.at(m_pageIndex).SetTextIndex(textIndex);
    if (m_SE != nullptr)
    {
        m_SE->PlayMove();
    }
    m_waitNextCount = 0;
}

bool QTE_Module::Update()
{
    if (m_pageList.empty())
    {
        return false;
    }
    InitConstValue();

    bool isFinish = false;
    if (m_isFadeIn)
    {
        if (m_FadeInCount < FADE_FRAME_MAX)
        {
            m_FadeInCount++;
        }
        else
        {
            m_isFadeIn = false;
            m_FadeInCount = 0;
        }
    }
    else if (m_isFadeOut)
    {
        if (m_FadeOutCount < FADE_FRAME_MAX)
        {
            m_FadeOutCount++;
        }
        else
        {
            m_isFadeOut = false;
            m_FadeOutCount = 0;
            m_isFadeIn = true;
            m_FadeInCount = 0;
            if (m_pageIndex <= (int)m_pageList.size() - 2)
            {
                m_pageIndex++;
                m_pageList.at(m_pageIndex).SetTextIndex(0);
            }
            else
            {
                isFinish = true;
            }
        }
    }
    else
    {
        m_waitNextCount++;
    }
    return isFinish;
}

void QTE_Module::Render()
{
    if (!m_pageList.empty())
    {
        m_pageList.at(m_pageIndex).GetSprite()->DrawImage(0, 0);
    }
    if (m_sprTextBack != nullptr)
    {
        m_sprTextBack->DrawImage(0, 0);
    }
    if (!m_pageList.empty() && m_font != nullptr)
    {
        std::vector<std::vector<std::wstring>> vss = m_pageList.at(m_pageIndex).GetTextList();
        int textIndex = m_pageList.at(m_pageIndex).GetTextIndex();
        if (vss.at(textIndex).size() >= 1)
        {
            m_font->DrawText_(vss.at(textIndex).at(0), 100, 730);
        }
        if (vss.at(textIndex).size() >= 2)
        {
            m_font->DrawText_(vss.at(textIndex).at(1), 100, 780);
        }
        if (vss.at(textIndex).size() >= 3)
        {
            m_font->DrawText_(vss.at(textIndex).at(2), 100, 830);
        }
    }

    if (m_sprWhiteBar != nullptr)
    {
        m_sprWhiteBar->DrawImage((m_screenWidth - 256) / 2, m_screenHeight / 2 - 20);
    }
    if (m_sprBlackBar != nullptr)
    {
        m_sprBlackBar->DrawImage((m_screenWidth - 256) / 2, m_screenHeight / 2 + 12);
    }

    if (m_sprFade != nullptr)
    {
        if (m_isFadeIn)
        {
            m_sprFade->DrawImage(0, 0, 255 - m_FadeInCount*255/FADE_FRAME_MAX);
        }
        if (m_isFadeOut)
        {
            m_sprFade->DrawImage(0, 0, m_FadeOutCount*255/FADE_FRAME_MAX);
        }
    }
}

void QTE_Module::Finalize()
{
    if (m_sprTextBack != nullptr)
    {
        delete m_sprTextBack;
        m_sprTextBack = nullptr;
    }
    if (m_sprFade != nullptr)
    {
        delete m_sprFade;
        m_sprFade = nullptr;
    }
    if (m_font != nullptr)
    {
        delete m_font;
        m_font = nullptr;
    }
    if (m_SE != nullptr)
    {
        delete m_SE;
        m_SE = nullptr;
    }
    for (std::size_t i = 0; i < m_pageList.size(); ++i)
    {
        delete m_pageList.at(i).GetSprite();
        m_pageList.at(i).SetSprite(nullptr);
    }
    if (m_sprImage != nullptr)
    {
        delete m_sprImage;
        m_sprImage = nullptr;
    }
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

void NS_QTE_Module::QTE_Module::SetFastMode(const bool arg)
{
    m_fastMode = arg;
}

void NS_QTE_Module::QTE_Module::OnDeviceLost()
{
    if (m_sprFade != nullptr)
    {
        m_sprFade->OnDeviceLost();
    }
    if (m_sprImage != nullptr)
    {
        m_sprImage->OnDeviceLost();
    }
    if (m_sprTextBack != nullptr)
    {
        m_sprTextBack->OnDeviceLost();
    }
    if (m_font != nullptr)
    {
        m_font->OnDeviceLost();
    }

    for (auto& item : m_pageList)
    {
        item.GetSprite()->OnDeviceLost();
    }

    if (m_sprWhiteBar != nullptr)
    {
        m_sprWhiteBar->OnDeviceLost();
    }
    if (m_sprBlackBar != nullptr)
    {
        m_sprBlackBar->OnDeviceLost();
    }
}

void NS_QTE_Module::QTE_Module::OnDeviceReset()
{
    if (m_sprFade != nullptr)
    {
        m_sprFade->OnDeviceReset();
    }
    if (m_sprImage != nullptr)
    {
        m_sprImage->OnDeviceReset();
    }
    if (m_sprTextBack != nullptr)
    {
        m_sprTextBack->OnDeviceReset();
    }
    if (m_font != nullptr)
    {
        m_font->OnDeviceReset();
    }

    for (auto& item : m_pageList)
    {
        item.GetSprite()->OnDeviceReset();
    }

    if (m_sprWhiteBar != nullptr)
    {
        m_sprWhiteBar->OnDeviceReset();
    }
    if (m_sprBlackBar != nullptr)
    {
        m_sprBlackBar->OnDeviceReset();
    }
}

void NS_QTE_Module::QTE_Module::InitConstValue()
{
    int& fade_frame_max = const_cast<int&>(FADE_FRAME_MAX);
    int& wait_next_frame = const_cast<int&>(WAIT_NEXT_FRAME);

    if (m_fastMode)
    {
        fade_frame_max = 1;
        wait_next_frame = 1;
    }
    else
    {
        fade_frame_max = 20;
        wait_next_frame = 10;
    }
}

void NS_QTE_Module::QTE_Module::SetBars(ISprite* whiteBar, ISprite* blackBar, int screenWidth, int screenHeight)
{
    m_sprWhiteBar = whiteBar;
    m_sprBlackBar = blackBar;
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
}

ISprite* Page::GetSprite() const
{
    return m_sprite;
}

void Page::SetSprite(ISprite* sprite)
{
    m_sprite = sprite;
}

std::vector<std::vector<std::wstring>> Page::GetTextList() const
{
    return m_textList;
}

void Page::SetTextList(const std::vector<std::vector<std::wstring>>& textList)
{
    m_textList = textList;
}

int Page::GetTextIndex() const
{
    return m_textIndex;
}

void Page::SetTextIndex(const int index)
{
    m_textIndex = index;
}

