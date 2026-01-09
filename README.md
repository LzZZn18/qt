# Talk文件夹监控程序

自动监控桌面上的`talk`文件夹，将新增的MP3文件转录为文本并上传到Dify知识库。

## 快速开始

1. **安装依赖**
bash
pip install -r requirements.txt

2. **配置API密钥**
编辑`.env`文件：
env
DIFY_API_KEY=你的Dify_API密钥
DIFY_KB_ID=你的知识库ID

3. **运行程序**