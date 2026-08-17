# ClassAssistant 班级助手

### 设计功能
- [ ] 课件整理（主要）
- [ ] 课件分类以及复制等快捷使用的功能
- [ ] 小组件（随机数 等）（插件形式 + 插件市场）
- [ ] 语音识别
- [ ] 班级管理相关功能（待确定）
- [ ] 其它功能

### 使用的第三方库
- [Fluent-Qt](https://github.com/calvinhxx/Fluent-Qt)
- onnxruntime
- [cppjieba](https://github.com/yanyiwu/cppjieba)
- [docxcpp](https://github.com/yunxingluoyun/docxcpp)
- [pptx-reader](https://github.com/huanhuan0812/pptx-reader)
- vosk api
- tessract api
- Qt

### 架构图
- ##### 设计架构
```mermaid
graph LR
    subgraph "1. UI 用户交互与展示层 (UI Layer)"
        subgraph "四大核心导航页面 (Main Tabs)"
            UI_File[文件管理页<br/> 卡片展示/预览 ]
            UI_Organize[整理任务页<br/> 一键整理/进度 ]
            UI_Search[全局搜索页<br/> 文本/语音输入 ]
            UI_Setting[系统设置页<br/> 基本配置/模型选择 ]
        end
        
        subgraph "页面内组件与特殊功能"
            UI_Float[悬浮球/快捷菜单<br/> 全局触发 ]
            UI_PluginStore[插件市场子页面<br/> 位于设置页中 ]
            UI_QMLContainer[QML 插件渲染容器<br/> 双击卡片/触发后弹出 ]
        end
    end

    subgraph "2. 核心控制与意图理解层 (Controller & Intent)"
        Ctrl[MainController<br/> 统一调度与分发 ]
        Intent[意图理解引擎<br/> Jieba + 模糊匹配 ]
    end

    subgraph "3. 功能执行与基础服务层 (Services)"
        subgraph "本地文件与解析"
            FileOps[文件操作 Service<br/> 复制/移动/打开 ]
            FileParse[多格式解析 Service<br/> PPT/DOCX 提取 ]
            FileWatch[文件监控 Service]
        end
        
        subgraph "AI 推理服务"
            ModelFactory[AI 策略工厂]
            Model_Onnx[本地 TextCNN]
            Model_Ollama[本地 Ollama]
            Model_API[云端 API]
        end
        
        subgraph "插件扩展服务"
            PluginMgr[插件核心管理器<br/> 加载/卸载/通信 ]
            PluginAPI[插件宿主接口]
            JSRuntime[JS 脚本运行环境]
            DLLLoader[动态库加载器]
        end
    end

    subgraph "4. 全局基础设施层 (Infrastructure)"
        Settings[全局设置单例<br/> 记录当前模型选择 ]
        TempData[临时状态单例]
        Jieba[Jieba 分词单例]
    end

    subgraph "5. 数据持久化与资源层 (Data/Res)"
        FileSystem[本地文件与目录]
        IndexDB[索引与配置: json ]
        PluginCache[本地已安装插件<br/> .qml/.js/.dll ]
        PluginStore[远端插件市场 API]
    end

    %% 控制流 (四大页面输入)
    UI_File --> Ctrl
    UI_Organize --> Ctrl
    UI_Search --> Ctrl
    UI_Setting --> Ctrl
    
    UI_Float --> Ctrl
    UI_PluginStore --> Ctrl
    
    Ctrl --> Intent
    Ctrl --> FileOps
    Ctrl --> FileParse
    Ctrl --> FileWatch
    Ctrl --> ModelFactory
    Ctrl --> PluginMgr
    
    %% AI 并行策略
    ModelFactory -.-> Model_Onnx
    ModelFactory -.-> Model_Ollama
    ModelFactory -.-> Model_API
    
    %% 插件机制
    PluginMgr --> PluginAPI
    PluginMgr --> DLLLoader
    PluginMgr --> JSRuntime
    PluginAPI --> FileOps
    PluginAPI --> FileParse
    PluginAPI --> ModelFactory
    
    %% 数据流
    FileOps --> FileSystem
    FileParse --> IndexDB
    FileWatch --> FileSystem
    PluginMgr --> PluginCache
    
    %% 商店网络
    UI_PluginStore <--> PluginStore
    PluginStore --> PluginMgr
    
    %% 全局依赖
    Settings -.-> Ctrl
    Settings -.-> ModelFactory
    Jieba -.-> Intent
    TempData -.-> UI_File
    TempData -.-> UI_Search
```
- ##### c++ 类结构

```mermaid
classDiagram
    direction TB

    %% 核心单例
    class GlobalSettings { <<Singleton>> }
    class TempDataManager { <<Singleton>> }
    class JiebaEngine { <<Singleton>> }

    %% 主控制器
    class MainController {
        +onCardClicked(path)
        +onOrganizeClicked()
        +onSearchInput(text)
        +onSettingChanged()
        -IntentEngine* m_intent
    }

    %% 意图理解
    class IntentEngine {
        +parseInput(text)
    }

    %% ---------- 插件与市场 ----------
    class PluginManager {
        <<QObject>>
        +loadPlugin(name)
        +unloadPlugin(name)
        -QQmlEngine* m_qmlEngine
    }

    class PluginStoreService {
        +fetchPluginList()
        +downloadPlugin(id)
    }

    class IPluginInterface {
        <<Abstract Class>> (DLL 接口)
        +virtual bool init()
        +virtual void execute(params)
    }

    class DynamicLibLoader {
        +loadDLL(path)
    }

    class QmlHostWidget {
        <<QQuickWidget>>
        +loadQML(url)
        +callJSFunction(funcName, args)
    }

    %% 依赖关系
    MainController --> GlobalSettings
    MainController --> TempDataManager
    MainController *-- IntentEngine
    IntentEngine ..> JiebaEngine
    
    MainController *-- PluginManager
    MainController --> PluginStoreService : "设置在设置页触发"
    
    PluginManager --> QmlHostWidget
    PluginManager --> DynamicLibLoader
    PluginManager ..> IPluginInterface
```

- ##### 时序
```mermaid
sequenceDiagram
    autonumber
    actor User as 用户
    participant UI as 四大核心页面<br/>(文件/整理/搜索/设置)
    participant Ctrl as MainController
    participant Intent as IntentEngine
    participant AI as AI模型策略工厂
    participant Store as PluginStoreService
    participant PMgr as PluginManager
    participant Server as 远端插件市场
    participant FileSys as 本地文件系统

    %% 场景一：核心主流程 (文件页 -> 整理 -> AI)
    User->>UI: 1. 在【文件页】点击卡片
    UI->>Ctrl: 2. 触发文件整理/解析
    activate Ctrl
    Ctrl->>Intent: 3. parseIntent(文本摘要)
    activate Intent
    Intent-->>Ctrl: 4. 返回整理意图
    deactivate Intent
    Ctrl->>AI: 5. 调用当前选择的模型分类
    activate AI
    AI-->>Ctrl: 6. 返回 "课件" 分类
    deactivate AI
    Ctrl->>FileSys: 7. 将文件移至 /课件/ 目录
    FileSys-->>Ctrl: 8. 移动完成
    Ctrl->>UI: 9. 刷新【文件页】与【整理页】进度
    deactivate Ctrl

    %% 场景二：核心主流程 (搜索页 -> 意图)
    User->>UI: 10. 在【搜索页】输入 "2023年总结"
    UI->>Ctrl: 11. 触发搜索
    activate Ctrl
    Ctrl->>Intent: 12. parseIntent("2023年总结")
    activate Intent
    Intent-->>Ctrl: 13. 返回搜索指令 {时间:2023, 关键词:总结}
    deactivate Intent
    Ctrl->>FileSys: 14. 检索索引库
    FileSys-->>Ctrl: 15. 返回结果
    Ctrl->>UI: 16. 更新【搜索页】列表
    deactivate Ctrl

    %% 场景三：插件市场流程 (设置页 -> 市场入口)
    User->>UI: 17. 切换到【设置页】
    UI->>UI: 18. 用户点击 "插件市场" 子菜单
    UI->>Store: 19. 获取插件列表
    activate Store
    Store->>Server: 20. 网络请求 /plugin_list
    Server-->>Store: 21. 返回 JSON 市场列表
    Store-->>UI: 22. 渲染【设置页】内的插件商店UI
    deactivate Store
    
    User->>UI: 23. 点击 "安装 AI 扩展"
    UI->>Store: 24. 发起下载 /download/ai_ext
    activate Store
    Store->>Server: 25. 下载插件包
    Server-->>Store: 26. 返回二进制数据
    Store->>PMgr: 27. 解压并通知安装
    deactivate Store
    PMgr-->>UI: 28. 刷新【设置页】插件状态，提示安装成功
```

- ##### ui
```mermaid
graph TD
    %% 全局入口
    Entry[App 主入口] --> TabBar
    
    subgraph "核心导航栏 (TabBar)"
        direction LR
        TB_File[📁 文件页]
        TB_Organize[📋 整理页]
        TB_Search[🔍 搜索页]
        TB_Settings[⚙️ 设置页]
    end

    %% ================= 1. 文件页面：单列主视图 (主视觉) =================
    subgraph "主视图：文件页 (单列卡片布局)"
        direction TB
        FileView_Top[顶部：路径导航/搜索过滤栏]
        
        subgraph "单列卡片瀑布流 (Single Column)"
            direction TB
            Card_1[文件卡片 A<br/>PPTX / 课件]
            Card_2[文件卡片 B<br/>DOCX / 作业]
            Card_3[文件卡片 C<br/>图片 / 图纸]
            Card_4[文件卡片 D<br/>PDF / 笔记]
        end
        FileView_Top --> Card_1 --> Card_2 --> Card_3 --> Card_4
        
        Card_1 -->|单击/右键| ContextMenu[单卡片操作菜单<br/>- 打开<br/>- 重命名<br/>- 复制到【双列工具】]
    end

    %% ================= 2. 悬浮球触发：双列工具视图 (副视觉) =================
    subgraph "全局悬浮球 [FloatingBall]"
        Float_Ball[悬浮球] -->|点击| Float_Menu[全局快捷菜单]
        Float_Menu -->|选择 双栏文件管理 | Dual_View
    end

    subgraph "副视图：双列文件管理工具 (拖拽复制)"
        direction LR
        
        subgraph "左列：源文件夹 (Source / 当前浏览)"
            Left_Top[顶部：路径 / 源目录名]
            L_Card_1[文件卡片 1]
            L_Card_2[文件卡片 2]
            L_Card_3[文件卡片 3]
            Left_Top --> L_Card_1 --> L_Card_2 --> L_Card_3
        end
        
        subgraph "中间：拖拽交互区"
            Drag_Action((按住拖拽<br/>跨列移动))
        end
        
        subgraph "右列：目标文件夹 (Target / 备份或归类)"
            Right_Top[顶部：路径 / 目标目录名]
            R_Card_A[文件卡片 A]
            R_Card_B[文件卡片 B]
            Right_Top --> R_Card_A --> R_Card_B
        end
    end

    %% ================= 3. 交互动作流：跨列拖拽复制 =================
    L_Card_1 -->|触发长按/拖拽| Drag_Action
    Drag_Action -->|进入右列区域释放| Copy_Trigger
    
    subgraph "执行动作层 (Action Layer)"
        Copy_Trigger[触发复制/移动指令] --> Select_Action[选择动作：<br/>复制（Ctrl）/ 移动（Shift）]
        Select_Action --> File_Exec[系统文件操作<br/>QFile::copy / QFile::rename]
        File_Exec --> Refresh_Main[刷新当前卡片列表]
        Refresh_Main --> Card_1
        Refresh_Main --> L_Card_1
        Refresh_Main --> R_Card_A
    end

    %% ================= 4. 后续页面跳转递进 =================
    TabBar --> TB_Organize
    TabBar --> TB_Search
    TabBar --> TB_Settings
    
    %% 整理的递进
    TB_Organize -->|点击批量整理| Batch_Select[批量选择多卡片]
    Batch_Select --> Task_Queue[后台任务队列]
    Task_Queue -->|完成后| Result_Toast[弹窗提示整理结果]
    
    %% 搜索的递进
    TB_Search --> Search_Input[文本/语音输入框]
    Search_Input --> Search_Grid[搜索结果展示（网格布局）]
    Search_Grid -->|点击结果项| Open_File[调用系统打开]
    
    %% 设置的递进
   
```