# UE插件QuickAccessTool介绍

## 前言
简化UE操作势在必行

## 支持版本
UE4.27~ UE5.6

## 功能预览

### 快速面板

#### 打开插件 Ctrl + Q (或者点击插件)

![alt text](/Resources/OpenPlugin.gif)

#### 拖拽或者右键添加常用文件进快速面板

![alt text](/Resources/AddToQuickPanel.gif)

#### 快速面板双击文件或者右键菜单打开文件

![alt text](/Resources/OpenFile.gif)

#### 快速面板Ctrl + B或者右键菜单打开文件所在内容浏览器

![alt text](/Resources/OpenFileContent.gif)

#### 支持Ctrl单选与Shift多选操作

#### 其它操作看图

![alt text](/Resources/PanelFunction.png)

![alt text](/Resources/PanelFunctionChinese.png)

#### Alt + V 或者点击菜单操作可将剪切板图片变成Texture2D

![alt text](/Resources/PasteImage.gif)

---

### 通用面板

#### 通用面板涵盖切语言、设置后台用较少Cpu，打开取色器复制颜色到剪切板

![alt text](/Resources/CommonPanel.gif)

---

### 任务面板

#### 记录任务

![alt text](/Resources/TaskPanel.png)

#### 鼠标滚轮可放大缩小

![alt text](/Resources/WheelTask.gif)

## 缓存设置(QuickAccessSavePath)

![alt text](/Resources/Save.png)

## 版本对应需要编译问题
QuickAccessButton就是源代码的SButton,加了个OnDoubleClick事件,此部分如果版本不同报错,把源代码的SButton拷贝出来,加上DoubleClick事件即可(Slate继承之后,很多属性与事件需要转发,所以偷了个懒)

## 反馈
联系QQ2923211924并说明来意