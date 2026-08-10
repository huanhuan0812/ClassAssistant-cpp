// JiebaSingleton.h
#pragma once
#include <memory>
#include <string>
#include <set>
#include <mutex>
#include <QTimer>
#include <QObject>
#include <cppjieba/Jieba.hpp>

namespace TextProcessor {

/**
 * Jieba单例类 - 全局共享Jieba实例
 * 所有分词器共用同一个Jieba实例以节省内存
 * 支持闲置时自动卸载资源以节省内存（使用QTimer）
 */
class JiebaSingleton : public QObject {
    Q_OBJECT

public:
    // 获取单例实例
    static JiebaSingleton& getInstance();
    
    // 初始化Jieba
    bool initialize(
        const std::string& dict_path,
        const std::string& hmm_path,
        const std::string& user_dict_path,
        const std::string& idf_path,
        const std::string& stop_word_path
    );
    
    // 卸载Jieba实例（释放内存，保存配置以便恢复）
    bool unload();
    
    // 恢复Jieba实例（使用之前保存的配置重新加载）
    bool reload();
    
    // 检查是否已加载
    bool isLoaded() const { return jieba_ != nullptr && initialized_; }
    bool isReady() const { return jieba_ != nullptr && initialized_; }
    
    // 检查是否已初始化（包括已卸载但配置已保存的情况）
    bool isInitialized() const { return init_config_saved_; }
    
    // 获取Jieba实例（如果已卸载则自动恢复）
    cppjieba::Jieba* getJieba();
    
    // 获取停用词表（始终可用，即使已卸载）
    const std::set<std::string>& getStopwords() const { return stopwords_; }
    
    // 获取有意义的单字表（始终可用，即使已卸载）
    const std::set<std::string>& getMeaningfulSingleChars() const { return meaningful_single_chars_; }
    
    // 获取初始化错误信息
    std::string getInitError() const { return init_error_; }
    
    // ============ 路径管理方法 ============
    
    // 设置单个词典路径（不立即初始化）
    void setDictPath(const std::string& path);
    void setHmmPath(const std::string& path);
    void setUserDictPath(const std::string& path);
    void setIdfPath(const std::string& path);
    void setStopWordPath(const std::string& path);
    
    // 批量设置所有路径
    void setDictPaths(
        const std::string& dict_path,
        const std::string& hmm_path,
        const std::string& user_dict_path,
        const std::string& idf_path,
        const std::string& stop_word_path
    );
    
    // 获取单个词典路径
    std::string getDictPath() const;
    std::string getHmmPath() const;
    std::string getUserDictPath() const;
    std::string getIdfPath() const;
    std::string getStopWordPath() const;
    
    // 获取所有路径
    struct DictPaths {
        std::string dict_path;
        std::string hmm_path;
        std::string user_dict_path;
        std::string idf_path;
        std::string stop_word_path;
    };
    DictPaths getAllPaths() const;
    
    // 验证路径是否有效（文件存在且可读）
    bool validateDictPath(const std::string& path) const;
    bool validateAllPaths() const;
    std::string getValidationError() const { return validation_error_; }
    
    // 清空所有路径
    void clearAllPaths();
    
    // 检查路径是否已设置
    bool hasDictPath() const;
    bool hasAllPaths() const;
    
    // 使用当前设置的路径重新加载
    bool reloadWithCurrentPaths();
    
    // ============ 自动管理策略 ============
    
    // 设置自动恢复策略
    void setAutoReload(bool enable) { auto_reload_ = enable; }
    bool isAutoReloadEnabled() const { return auto_reload_; }
    
    // 设置自动卸载策略
    void setAutoUnload(bool enable);
    bool isAutoUnloadEnabled() const { return auto_unload_; }
    
    // 设置闲置超时时间（秒），0表示永不超时
    void setIdleTimeout(int seconds);
    int getIdleTimeout() const { return idle_timeout_seconds_; }
    
    // 获取当前闲置时长（秒）
    int getIdleSeconds() const;
    
    // 手动触摸（重置闲置计时器）
    void touch();
    
    // 重新加载超时配置（从settings读取）
    void reloadTimeoutConfig();
    
    // 禁止拷贝和赋值
    JiebaSingleton(const JiebaSingleton&) = delete;
    JiebaSingleton& operator=(const JiebaSingleton&) = delete;

private slots:
    // QTimer超时槽函数
    void onIdleTimeout();

private:
    JiebaSingleton();
    ~JiebaSingleton();
    
    // Jieba实例
    std::unique_ptr<cppjieba::Jieba> jieba_;
    
    // 停用词表（不随Jieba实例卸载）
    std::set<std::string> stopwords_;
    
    // 有意义的单字表（不随Jieba实例卸载）
    std::set<std::string> meaningful_single_chars_;
    
    // 状态标志
    bool initialized_ = false;        // Jieba实例是否已加载
    bool init_config_saved_ = false;  // 是否已保存初始化配置
    bool auto_reload_ = true;         // 默认启用自动恢复
    bool auto_unload_ = true;         // 默认启用自动卸载
    
    // 配置路径（用于恢复）
    std::string dict_path_;
    std::string hmm_path_;
    std::string user_dict_path_;
    std::string idf_path_;
    std::string stop_word_path_;
    
    // 闲置管理
    int idle_timeout_seconds_ = 300;  // 默认5分钟
    QTimer* idle_timer_ = nullptr;
    std::chrono::steady_clock::time_point last_access_time_;
    
    // 错误信息
    std::string init_error_;
    mutable std::string validation_error_;  // 路径验证错误信息
    
    // 线程安全
    mutable std::mutex mutex_;
    mutable std::mutex time_mutex_;
    
    // 内部方法
    bool doInitialize(
        const std::string& dict_path,
        const std::string& hmm_path,
        const std::string& user_dict_path,
        const std::string& idf_path,
        const std::string& stop_word_path
    );
    
    void initStopwords();
    void initMeaningfulSingleChars();
    bool checkDictFiles() const;
    bool checkSingleFile(const std::string& path, const std::string& name) const;
    void clearJieba();
    void updateAccessTime();
    void startIdleTimer();
    void stopIdleTimer();
    void resetIdleTimer();
    int getTimeoutFromSettings() const;
};

} // namespace TextProcessor