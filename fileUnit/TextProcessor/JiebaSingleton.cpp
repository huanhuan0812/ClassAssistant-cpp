// JiebaSingleton.cpp
#include "JiebaSingleton.h"
#include "../../settings.h"
#include <fstream>
#include <iostream>
#include <QCoreApplication>

namespace TextProcessor {

JiebaSingleton::JiebaSingleton() : QObject(nullptr) {
    // 从settings读取超时配置
    idle_timeout_seconds_ = getTimeoutFromSettings();
    
    // 创建定时器（在Qt事件循环中运行）
    idle_timer_ = new QTimer(this);
    idle_timer_->setSingleShot(true);  // 单次触发
    connect(idle_timer_, &QTimer::timeout, this, &JiebaSingleton::onIdleTimeout);
    
    std::cout << "JiebaSingleton: 初始化完成，闲置超时 = " 
              << idle_timeout_seconds_ << " 秒" << std::endl;
}

JiebaSingleton::~JiebaSingleton() {
    if (idle_timer_) {
        idle_timer_->stop();
    }
    clearJieba();
}

JiebaSingleton& JiebaSingleton::getInstance() {
    static JiebaSingleton instance;
    return instance;
}

int JiebaSingleton::getTimeoutFromSettings() const {
    try {
        auto& settings = settings::instance();
        return settings.jiebaIdleTimeoutSeconds;
    } catch (const std::exception& e) {
        std::cerr << "JiebaSingleton: 读取settings失败，使用默认值300秒" << std::endl;
        return 300;  // 默认5分钟
    }
}

void JiebaSingleton::reloadTimeoutConfig() {
    std::lock_guard<std::mutex> lock(mutex_);
    int new_timeout = getTimeoutFromSettings();
    if (new_timeout != idle_timeout_seconds_) {
        idle_timeout_seconds_ = new_timeout;
        std::cout << "JiebaSingleton: 超时配置已更新为 " 
                  << idle_timeout_seconds_ << " 秒" << std::endl;
        
        // 如果已加载且自动卸载启用，重启定时器
        if (initialized_ && jieba_ != nullptr && auto_unload_) {
            resetIdleTimer();
        }
    }
}

bool JiebaSingleton::initialize(
    const std::string& dict_path,
    const std::string& hmm_path,
    const std::string& user_dict_path,
    const std::string& idf_path,
    const std::string& stop_word_path
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 重新读取超时配置
    idle_timeout_seconds_ = getTimeoutFromSettings();
    
    // 如果已经初始化且已加载，直接返回
    if (initialized_ && jieba_ != nullptr) {
        updateAccessTime();
        resetIdleTimer();
        return true;
    }
    
    // 如果已卸载但配置已保存，更新配置并重新加载
    if (init_config_saved_) {
        dict_path_ = dict_path;
        hmm_path_ = hmm_path;
        user_dict_path_ = user_dict_path;
        idf_path_ = idf_path;
        stop_word_path_ = stop_word_path;
        bool result = doInitialize(dict_path, hmm_path, user_dict_path, idf_path, stop_word_path);
        if (result) {
            updateAccessTime();
            resetIdleTimer();
        }
        return result;
    }
    
    // 首次初始化
    bool result = doInitialize(dict_path, hmm_path, user_dict_path, idf_path, stop_word_path);
    if (result) {
        updateAccessTime();
        resetIdleTimer();
    }
    return result;
}

bool JiebaSingleton::doInitialize(
    const std::string& dict_path,
    const std::string& hmm_path,
    const std::string& user_dict_path,
    const std::string& idf_path,
    const std::string& stop_word_path
) {
    // 保存配置
    dict_path_ = dict_path;
    hmm_path_ = hmm_path;
    user_dict_path_ = user_dict_path;
    idf_path_ = idf_path;
    stop_word_path_ = stop_word_path;
    
    // 检查词典文件
    if (!checkDictFiles()) {
        return false;
    }
    
    try {
        jieba_ = std::make_unique<cppjieba::Jieba>(
            dict_path, hmm_path, user_dict_path, idf_path, stop_word_path
        );
        initialized_ = true;
        init_config_saved_ = true;
        init_error_.clear();
        std::cout << "JiebaSingleton: Jieba实例已加载" << std::endl;
    } catch (const std::exception& e) {
        init_error_ = "Jieba 初始化失败: " + std::string(e.what());
        initialized_ = false;
        init_config_saved_ = false;
        std::cerr << "JiebaSingleton 错误: " << init_error_ << std::endl;
        jieba_ = nullptr;
        return false;
    }
    
    // 初始化停用词和有意义单字（仅首次）
    if (stopwords_.empty()) {
        initStopwords();
    }
    if (meaningful_single_chars_.empty()) {
        initMeaningfulSingleChars();
    }
    
    return true;
}

bool JiebaSingleton::checkDictFiles() const {
    std::vector<std::pair<std::string, std::string>> files = {
        {"jieba.dict.utf8", dict_path_},
        {"hmm_model.utf8", hmm_path_},
        {"user.dict.utf8", user_dict_path_},
        {"idf.utf8", idf_path_},
        {"stop_words.utf8", stop_word_path_}
    };
    
    for (const auto& [name, path] : files) {
        std::ifstream file(path);
        if (!file.is_open()) {
            const_cast<JiebaSingleton*>(this)->init_error_ = 
                "词典文件不存在: " + path + " (" + name + ")";
            std::cerr << "JiebaSingleton 错误: " << init_error_ << std::endl;
            return false;
        }
    }
    return true;
}

void JiebaSingleton::clearJieba() {
    if (jieba_) {
        jieba_.reset();
    }
    initialized_ = false;
}

bool JiebaSingleton::unload() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || jieba_ == nullptr) {
        // 已经卸载或未初始化
        stopIdleTimer();
        return true;
    }
    
    try {
        clearJieba();
        init_error_ = "Jieba实例已卸载（可恢复）";
        stopIdleTimer();
        std::cout << "JiebaSingleton: 已卸载Jieba实例" << std::endl;
        return true;
    } catch (const std::exception& e) {
        init_error_ = "卸载Jieba失败: " + std::string(e.what());
        std::cerr << "JiebaSingleton 错误: " << init_error_ << std::endl;
        return false;
    }
}

bool JiebaSingleton::reload() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 如果已经加载，先卸载
    if (initialized_ && jieba_ != nullptr) {
        clearJieba();
    }
    
    if (!init_config_saved_) {
        init_error_ = "无法恢复：未保存初始化配置";
        std::cerr << "JiebaSingleton 错误: " << init_error_ << std::endl;
        return false;
    }
    
    // 重新加载
    bool result = doInitialize(
        dict_path_, hmm_path_, user_dict_path_, idf_path_, stop_word_path_
    );
    if (result) {
        updateAccessTime();
        resetIdleTimer();
    }
    return result;
}

void JiebaSingleton::updateAccessTime() {
    std::lock_guard<std::mutex> lock(time_mutex_);
    last_access_time_ = std::chrono::steady_clock::now();
}

int JiebaSingleton::getIdleSeconds() const {
    std::lock_guard<std::mutex> lock(time_mutex_);
    auto now = std::chrono::steady_clock::now();
    return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now - last_access_time_).count());
}

void JiebaSingleton::startIdleTimer() {
    if (idle_timer_ && auto_unload_ && idle_timeout_seconds_ > 0 && 
        initialized_ && jieba_ != nullptr) {
        idle_timer_->start(idle_timeout_seconds_ * 1000);  // 转换为毫秒
        std::cout << "JiebaSingleton: 启动闲置定时器，超时时间 " 
                  << idle_timeout_seconds_ << " 秒" << std::endl;
    }
}

void JiebaSingleton::stopIdleTimer() {
    if (idle_timer_) {
        idle_timer_->stop();
    }
}

void JiebaSingleton::resetIdleTimer() {
    stopIdleTimer();
    if (initialized_ && jieba_ != nullptr && auto_unload_) {
        startIdleTimer();
    }
}

void JiebaSingleton::onIdleTimeout() {
    // 检查闲置时间
    std::lock_guard<std::mutex> lock(time_mutex_);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_access_time_).count();
    
    // 如果已经卸载或超时时间无效，直接返回
    if (!initialized_ || jieba_ == nullptr || !auto_unload_ || idle_timeout_seconds_ <= 0) {
        return;
    }
    
    // 检查是否超时
    if (elapsed >= idle_timeout_seconds_) {
        std::cout << "JiebaSingleton: 闲置超时 (" << elapsed << "秒)，自动卸载" << std::endl;
        // 解锁后卸载（避免死锁）
        time_mutex_.unlock();
        unload();
        time_mutex_.lock();
    } else if (idle_timer_ && auto_unload_) {
        // 重新设置定时器，继续等待
        int remaining = idle_timeout_seconds_ - static_cast<int>(elapsed);
        if (remaining > 0) {
            idle_timer_->start(remaining * 1000);
        }
    }
}

cppjieba::Jieba* JiebaSingleton::getJieba() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 如果未加载但配置已保存且启用了自动恢复
    if ((!initialized_ || jieba_ == nullptr) && init_config_saved_ && auto_reload_) {
        std::cout << "JiebaSingleton: 自动恢复Jieba实例" << std::endl;
        mutex_.unlock();
        bool success = reload();
        mutex_.lock();
        if (success) {
            updateAccessTime();
            resetIdleTimer();
            return jieba_.get();
        }
        return nullptr;
    }
    
    if (initialized_ && jieba_ != nullptr) {
        updateAccessTime();
        resetIdleTimer();  // 每次访问重置定时器
    }
    
    return jieba_.get();
}

void JiebaSingleton::touch() {
    updateAccessTime();
    resetIdleTimer();
}

void JiebaSingleton::setAutoUnload(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto_unload_ = enable;
    if (enable) {
        resetIdleTimer();
    } else {
        stopIdleTimer();
    }
    std::cout << "JiebaSingleton: 自动卸载 " << (enable ? "启用" : "禁用") << std::endl;
}

void JiebaSingleton::setIdleTimeout(int seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    idle_timeout_seconds_ = seconds;
    
    // 同时更新settings
    try {
        auto& settings = settings::instance();
        settings.jiebaIdleTimeoutSeconds = seconds;
        settings.save();  // 保存到配置文件
    } catch (const std::exception& e) {
        std::cerr << "JiebaSingleton: 保存settings失败: " << e.what() << std::endl;
    }
    
    if (seconds > 0 && auto_unload_ && initialized_ && jieba_ != nullptr) {
        // 重启定时器
        stopIdleTimer();
        startIdleTimer();
    } else if (seconds <= 0) {
        stopIdleTimer();
    }
    std::cout << "JiebaSingleton: 闲置超时设置为 " << seconds << " 秒" << std::endl;
}

void JiebaSingleton::initStopwords() {
    std::vector<std::string> stop_words = {
        "的", "了", "是", "我", "你", "他", "她", "它", "我们", "你们", "他们",
        "这", "那", "有", "在", "不", "和", "与", "就", "都", "而", "及", "或",
        "一个", "这个", "那个", "那些", "这些", "这里", "那里", "然后", "因为",
        "所以", "但是", "如果", "虽然", "然而", "并且", "或者"
    };
    stopwords_.insert(stop_words.begin(), stop_words.end());
}

void JiebaSingleton::initMeaningfulSingleChars() {
    std::vector<std::string> chars = {
        "圆", "力", "氧", "氢", "碳", "钠", "酸", "碱", "盐",
        "电", "光", "声", "热", "诗", "词", "歌", "曲", "数",
        "方", "程", "函", "数", "角", "形", "体", "积"
    };
    meaningful_single_chars_.insert(chars.begin(), chars.end());
}

} // namespace TextProcessor