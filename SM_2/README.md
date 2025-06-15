# 专业英语学习程序

## 项目简介
这是一个基于SM-2间隔重复算法的英语单词学习程序，专注于集成电路专业英语词汇的学习。程序采用C语言开发，实现了基于会话(session)的自适应复习计划，帮助用户高效地记忆和掌握专业词汇。

## 主要功能
1. **单词学习**
   - 智能复习计划：基于改进的SM-2算法自动安排复习时间
   - 会话管理：基于session的学习进度跟踪
   - 答案评判：多级评分系统（0-5分）
   - 进度保存：自动保存学习进度和会话信息

2. **学习统计**
   - 显示总单词数
   - 已掌握单词数量（复习次数≥3）
   - 正在学习的单词数量
   - 未开始学习的单词数量
   - 错误次数统计

3. **进度追踪**
   - 记录每个单词的复习次数
   - 跟踪错误次数
   - 显示下次复习会话
   - 记录学习难度因子

## 技术特点
1. **改进的SM-2算法实现**
   - 动态调整复习间隔
   - 自适应难度因子（1.3-2.5范围）
   - 基于会话的间隔计算
   - 多级评分系统

2. **数据持久化**
   - 单词库文件（words.txt）
   - 学习进度文件（progress.txt）
   - 会话记录文件（session.txt）

3. **内存管理**
   - 固定大小的单词数组（最大1000词）
   - 高效的单词检索
   - 安全的字符串处理
   - 会话状态管理

## 文件结构
SM_2/
├── SM_2/
│ ├── main.c          # 主程序入口
│ ├── word.h          # 单词系统头文件
│ ├── word.c          # 单词系统实现
│ ├── flashcard.h     # 闪卡系统头文件
│ ├── flashcard.c     # 闪卡系统实现
│ ├── flashcard_grader.h  # SM-2评分系统头文件
│ ├── flashcard_grader.c  # SM-2评分系统实现
│ ├── learning.h      # 学习系统头文件
│ ├── learning.c      # 学习系统实现
│ ├── statistics.h    # 统计系统头文件
│ ├── statistics.c    # 统计系统实现
│ ├── wordbookload.c  # 文件加载保存实现
│ ├── words.txt       # 单词库文件
│ ├── progress.txt    # 进度保存文件
│ └── session.txt     # 会话记录文件

## 使用说明
1. **编译程序**
   - 使用Visual Studio 2022打开项目
   - 选择x64平台
   - 编译生成可执行文件

2. **运行程序**
   - 确保words.txt在程序同目录下
   - 运行生成的可执行文件
   - 按照菜单提示操作

3. **学习流程**
   - 选择"开始单词学习"
   - 根据中文提示输入英文单词
   - 系统显示答案正确与否
   - 显示详细学习状态（复习次数、间隔、难度因子等）
   - 按Enter继续，按'q'退出

## 数据文件格式
1. **words.txt**
   ```
   word    meaning
   software    软件
   computer    计算机
   ```

2. **progress.txt**
   ```
   word    meaning    repetition    interval    easiness_factor    session_count    next_session
   ```

3. **session.txt**
   ```
   current_session_number
   ```

## 算法说明
程序使用改进的SM-2算法：

1. 答对（grade>=3）时：
   - 首次答对：interval = 1, repetition = 1
   - 第二次答对：interval = 2, repetition = 2
   - 之后答对：interval = interval * easiness_factor（如果新间隔小于等于当前间隔，则加1）

2. 答错（grade<3）时：
   - 重置interval为1
   - 重置repetition为0
   - 降低easiness_factor
   - 下次学习立即复习

3. easiness_factor的计算：
   - 更新公式：EF = EF + (0.1 - (5-grade) * (0.08 + (5-grade) * 0.02))
   - 限制范围：1.3 <= EF <= 2.5

4. 会话管理：
   - 使用session计数跟踪学习进度
   - next_session = current_session + interval
   - 每次启动程序时session增加
   - 自动保存当前session进度

## 注意事项
1. 请勿手动修改progress.txt和session.txt
2. 确保程序有文件读写权限
3. 建议每次学习完成后使用'q'正常退出
4. 程序会自动保存学习进度和会话状态

## 未来计划
1. 实现句子学习功能
2. 添加学习数据可视化
3. 支持多用户配置
4. 添加复习提醒功能

## 贡献指南
欢迎提交Issue和Pull Request来帮助改进程序。在提交代码前，请确保：
1. 代码符合现有的编码风格
2. 所有新功能都有适当的测试
3. 更新了相关文档



