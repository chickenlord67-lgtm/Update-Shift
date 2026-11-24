// update-shift | main.cpp
// target: GD 2.206 | android arm64 + windows x64

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

enum class GDVersion {
    V1_0, V1_1, V1_2, V1_3, V1_4,
    V1_5, V1_6, V1_7, V1_8, V1_9,
    V2_0, V2_1, V2_2
};

static const std::vector<std::string> VERSION_NAMES = {
    "1.0","1.1","1.2","1.3","1.4",
    "1.5","1.6","1.7","1.8","1.9",
    "2.0","2.1","2.2"
};

struct VersionProfile {
    std::string name;
    bool hasShip;
    bool hasUFO;
    bool hasBall;
    bool hasWave;
    bool hasRobot;
    bool hasSpider;
    bool hasSwing;
    bool hasPlatformer;
    bool hasDashOrb;
    bool hasCameraTriggers;
    int  maxObjectTab;
    int  objectLimit;
};

static const std::vector<VersionProfile> PROFILES = {
    { "1.0", false, false, false, false, false, false, false, false, false, false, 1, 2000  },
    { "1.1", true,  false, false, false, false, false, false, false, false, false, 1, 2000  },
    { "1.2", true,  true,  false, false, false, false, false, false, false, false, 2, 2000  },
    { "1.3", true,  true,  true,  false, false, false, false, false, false, false, 2, 2000  },
    { "1.4", true,  true,  true,  false, false, false, false, false, false, false, 3, 3000  },
    { "1.5", true,  true,  true,  true,  false, false, false, false, false, false, 3, 3000  },
    { "1.6", true,  true,  true,  true,  true,  false, false, false, false, false, 4, 4000  },
    { "1.7", true,  true,  true,  true,  true,  true,  false, false, false, false, 4, 4000  },
    { "1.8", true,  true,  true,  true,  true,  true,  false, false, false, false, 5, 6000  },
    { "1.9", true,  true,  true,  true,  true,  true,  false, false, false, false, 6, 8000  },
    { "2.0", true,  true,  true,  true,  true,  true,  false, false, false, false, 7, 10000 },
    { "2.1", true,  true,  true,  true,  true,  true,  false, false, true,  false, 8, 40000 },
    { "2.2", true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  9, 99999 },
};

static GDVersion g_currentVersion = GDVersion::V2_2;

const VersionProfile& getProfile() {
    return PROFILES[static_cast<int>(g_currentVersion)];
}

class VersionPickerPopup : public geode::Popup<> {
protected:
    bool setup() override {
        this->setTitle("Update Shift");

        auto winSize = CCDirector::get()->getWinSize();
        float yOffset = (VERSION_NAMES.size() - 1) * 22.f / 2.f;

        for (int i = (int)VERSION_NAMES.size() - 1; i >= 0; i--) {
            auto btn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create(
                    VERSION_NAMES[i].c_str(), 80, true, "bigFont.fnt",
                    (g_currentVersion == static_cast<GDVersion>(i))
                        ? "GJ_button_02.png"
                        : "GJ_button_01.png",
                    30.f, 0.8f
                ),
                this,
                menu_selector(VersionPickerPopup::onVersionSelect)
            );
            btn->setTag(i);

            auto menu = CCMenu::create();
            menu->setPosition({
                winSize.width / 2,
                winSize.height / 2 - 20 + yOffset - ((VERSION_NAMES.size() - 1 - i) * 22.f)
            });
            menu->addChild(btn);
            this->m_mainLayer->addChild(menu);
        }

        return true;
    }

    void onVersionSelect(CCObject* sender) {
        int idx = sender->getTag();
        g_currentVersion = static_cast<GDVersion>(idx);
        Mod::get()->setSavedValue("selected_version", idx);

        Notification::create(
            fmt::format("Version set to {}", PROFILES[idx].name),
            NotificationIcon::Success
        )->show();

        this->onClose(nullptr);

        if (auto editor = LevelEditorLayer::get()) {
            if (auto ui = editor->m_editorUI) {
                ui->reloadItemsInBar(ui->m_tabsArray, ui->m_selectedMode, true);
            }
        }
    }

public:
    static VersionPickerPopup* create() {
        auto ret = new VersionPickerPopup();
        if (ret->initAnchored(220.f, 340.f)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

class $modify(MyEditorUI, EditorUI) {
    bool init(LevelEditorLayer* lel) {
        if (!EditorUI::init(lel)) return false;

        int saved = Mod::get()->getSavedValue<int>("selected_version", 12);
        g_currentVersion = static_cast<GDVersion>(std::clamp(saved, 0, 12));

        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Ver", 40, true, "bigFont.fnt", "GJ_button_05.png", 25.f, 0.6f),
            this,
            menu_selector(MyEditorUI::onVersionPicker)
        );

        auto menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition({
            CCDirector::get()->getWinSize().width - 30.f,
            CCDirector::get()->getWinSize().height - 30.f
        });
        this->addChild(menu, 10);

        applyVersionRestrictions();
        return true;
    }

    void onVersionPicker(CCObject*) {
        VersionPickerPopup::create()->show();
    }

    void applyVersionRestrictions() {
        auto& p = getProfile();

        if (m_shipButton)   { m_shipButton->setVisible(p.hasShip);     m_shipButton->setEnabled(p.hasShip);   }
        if (m_ballButton)   { m_ballButton->setVisible(p.hasBall);     m_ballButton->setEnabled(p.hasBall);   }
        if (m_ufoButton)    { m_ufoButton->setVisible(p.hasUFO);       m_ufoButton->setEnabled(p.hasUFO);     }
        if (m_waveButton)   { m_waveButton->setVisible(p.hasWave);     m_waveButton->setEnabled(p.hasWave);   }
        if (m_robotButton)  { m_robotButton->setVisible(p.hasRobot);   m_robotButton->setEnabled(p.hasRobot); }
        if (m_spiderButton) { m_spiderButton->setVisible(p.hasSpider); m_spiderButton->setEnabled(p.hasSpider); }
        if (m_swingButton)  { m_swingButton->setVisible(p.hasSwing);   m_swingButton->setEnabled(p.hasSwing); }

        if (m_platformerToggle) {
            m_platformerToggle->setVisible(p.hasPlatformer);
            m_platformerToggle->setEnabled(p.hasPlatformer);
        }
    }

    void onSelectBuildTab(int tab) {
        auto& p = getProfile();
        if (tab > p.maxObjectTab) {
            Notification::create(
                fmt::format("Tab locked in version {}", p.name),
                NotificationIcon::Warning
            )->show();
            return;
        }
        EditorUI::onSelectBuildTab(tab);
    }
};

class $modify(MyLevelEditorLayer, LevelEditorLayer) {
    GameObject* createObject(int id, cocos2d::CCPoint pos, bool undo) {
        auto& p = getProfile();
        int currentCount = m_objects->count();

        if (currentCount >= p.objectLimit) {
            Notification::create(
                fmt::format("Object limit reached for version {} ({})",
                    p.name, p.objectLimit),
                NotificationIcon::Warning
            )->show();
            return nullptr;
        }
        return LevelEditorLayer::createObject(id, pos, undo);
    }
};
