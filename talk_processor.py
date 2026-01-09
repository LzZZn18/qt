#!/usr/bin/env python3
"""
Talk文件夹监控程序 - 修复繁简转换bug
"""

import os
import sys
import time
import json
import logging
from pathlib import Path
from datetime import datetime
from dotenv import load_dotenv

# 先加载环境变量
load_dotenv()

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(sys.stdout),
        logging.FileHandler('talk_processor.log', encoding='utf-8')
    ]
)
logger = logging.getLogger(__name__)

class TalkProcessor:
    def __init__(self):
        """初始化配置"""
        # 基本配置
        self.desktop = Path.home() / "Desktop"
        self.talk_folder = Path(os.getenv("TALK_FOLDER", str(self.desktop / "talk")))
        self.processed_folder = self.talk_folder / "processed"
        self.text_folder = self.talk_folder / "text"
        self.history_file = self.talk_folder / ".processed.json"
        
        # 创建文件夹
        for folder in [self.talk_folder, self.processed_folder, self.text_folder]:
            folder.mkdir(exist_ok=True, parents=True)
        
        # Whisper配置
        self.model_name = os.getenv("WHISPER_MODEL", "base")
        self.language = os.getenv("LANGUAGE", "zh")
        
        # Dify配置
        self.dify_api_key = os.getenv("DIFY_API_KEY")
        self.dify_kb_id = os.getenv("DIFY_KB_ID")
        self.dify_base_url = os.getenv("DIFY_API_URL", "https://api.dify.ai/v1").rstrip('/')
        
        # 繁简转换配置
        self.convert_to_simplified = True
        self.converter = None
        
        # 加载模型
        self.model = None
        self.processed_files = self.load_processed_history()
        
        # 验证配置
        self.verify_config()
    
    def verify_config(self):
        """验证Dify配置"""
        if not self.dify_api_key or self.dify_api_key == "your-dify-api-key-here":
            logger.warning("⚠️ Dify API密钥未配置，将跳过上传步骤")
            return False
        
        if not self.dify_kb_id or self.dify_kb_id == "your-knowledge-base-id":
            logger.warning("⚠️ 知识库ID未配置，将跳过上传步骤")
            return False
        
        logger.info("✅ Dify配置验证通过")
        logger.info(f"   繁简转换: {'开启' if self.convert_to_simplified else '关闭'}")
        return True
    
    def load_whisper_model(self):
        """延迟加载Whisper模型"""
        if self.model is None:
            try:
                import whisper
                logger.info(f"正在加载Whisper模型: {self.model_name}")
                self.model = whisper.load_model(self.model_name)
                logger.info("✅ 模型加载完成")
            except Exception as e:
                logger.error(f"❌ 加载模型失败: {e}")
                sys.exit(1)
        return self.model
    
    def load_converter(self):
        """延迟加载繁简转换器 - 修复版"""
        if self.converter is None and self.convert_to_simplified:
            try:
                import opencc
                # 🔥 修复：使用正确的配置文件名
                self.converter = opencc.OpenCC('t2s')  # 去掉.json后缀
                logger.info("✅ 繁简转换器加载完成")
            except ImportError:
                logger.warning("⚠️ 未安装opencc库，使用基础繁简转换")
                self.converter = "simple"
            except Exception as e:
                logger.warning(f"⚠️ 繁简转换器加载失败: {e}")
                # 尝试其他配置文件名
                self.converter = self.try_alternative_configs()
        return self.converter
    
    def try_alternative_configs(self):
        """尝试其他可能的配置文件"""
        try:
            import opencc
            # 尝试不同的配置文件名
            configs = ['t2s', 't2s.json', 'tw2sp', 'hk2s', 's2t', 't2tw']
            
            for config in configs:
                try:
                    converter = opencc.OpenCC(config)
                    logger.info(f"✅ 使用转换配置: {config}")
                    return converter
                except Exception as config_error:
                    logger.debug(f"配置 {config} 失败: {config_error}")
                    continue
            
            # 如果所有配置都失败，使用基础转换
            logger.warning("⚠️ 所有opencc配置尝试失败，使用基础转换")
            return "simple"
            
        except Exception as e:
            logger.warning(f"⚠️ 备用配置尝试失败: {e}")
            return "simple"
    
    def traditional_to_simplified(self, text):
        """繁体中文转换为简体中文"""
        if not text or not self.convert_to_simplified:
            return text
        
        converter = self.load_converter()
        
        if converter is None:
            return text
        
        try:
            if converter == "simple":
                # 使用基础字符替换
                return self.simple_traditional_to_simplified(text)
            else:
                # 使用opencc进行转换
                simplified_text = converter.convert(text)
                original_length = len(text)
                new_length = len(simplified_text)
                logger.debug(f"繁简转换: {original_length} → {new_length} 字符")
                return simplified_text
        except Exception as e:
            logger.warning(f"⚠️ 繁简转换失败: {e}")
            return text
    
    def simple_traditional_to_simplified(self, text):
        """基础繁简转换（常用字符替换）"""
        # 常用繁体到简体的字符映射
        traditional_to_simple = {
            '麼': '么', '為': '为', '們': '们', '個': '个', '會': '会',
            '時': '时', '過': '过', '發': '发', '電': '电', '話': '话',
            '國': '国', '學': '学', '經': '经', '電': '电', '腦': '脑',
            '體': '体', '製': '制', '動': '动', '畫': '画', '圖': '图',
            '書': '书', '見': '见', '來': '来', '這': '这', '個': '个',
            '樣': '样', '開': '开', '關': '关', '點': '点', '後': '后',
            '麵': '面', '裏': '里', '隻': '只', '鐘': '钟', '闆': '板',
            '準': '准', '幾': '几', '係': '系', '復': '复', '臺': '台',
            '灣': '湾', '馬': '马', '鳥': '鸟', '魚': '鱼', '龍': '龙',
            '鳳': '凤', '龜': '龟', '歲': '岁', '樂': '乐', '車': '车',
            '東': '东', '門': '门', '間': '间', '長': '长', '風': '风',
            '雲': '云', '飛': '飞', '頭': '头', '飯': '饭', '館': '馆',
            '樓': '楼', '師': '师', '醫': '医', '藥': '药', '讀': '读',
            '寫': '写', '說': '说', '話': '话', '語': '语', '課': '课',
            '試': '试', '題': '题', '難': '难', '簡': '简', '單': '单',
            '雙': '双', '對': '对', '錯': '错', '真': '真', '假': '假',
            '新': '新', '舊': '旧', '高': '高', '低': '低', '大': '大',
            '小': '小', '多': '多', '少': '少', '遠': '远', '近': '近',
            '快': '快', '慢': '慢', '熱': '热', '冷': '冷', '紅': '红',
            '黃': '黄', '藍': '蓝', '綠': '绿', '黑': '黑', '白': '白',
            '好': '好', '壞': '坏', '美': '美', '醜': '丑', '強': '强',
            '弱': '弱', '硬': '硬', '軟': '软', '乾': '干', '濕': '湿',
            '輕': '轻', '重': '重', '早': '早', '晚': '晚', '春': '春',
            '夏': '夏', '秋': '秋', '冬': '冬', '你': '你', '我': '我',
            '他': '他', '她': '她', '它': '它', '是': '是', '不': '不',
            '有': '有', '沒': '没', '在': '在', '的': '的', '了': '了',
            '着': '着', '過': '过', '啊': '啊', '呀': '呀', '呢': '呢',
            '吧': '吧', '嗎': '吗', '啦': '啦', '喔': '喔',
        }
        
        # 逐个字符替换
        simplified_text = ''.join([
            traditional_to_simple.get(char, char) for char in text
        ])
        
        logger.debug("使用基础繁简转换完成")
        return simplified_text
    
    def clean_text(self, text):
        """清理文本内容"""
        if not text:
            return ""
        
        import re
        text = re.sub(r'\s+', ' ', text.strip())
        return text
    
    def load_processed_history(self):
        """加载已处理文件历史"""
        if self.history_file.exists():
            try:
                with open(self.history_file, 'r', encoding='utf-8') as f:
                    return set(json.load(f))
            except Exception as e:
                logger.warning(f"读取处理历史失败: {e}")
                return set()
        return set()
    
    def save_processed_history(self, filename):
        """保存处理记录"""
        self.processed_files.add(filename)
        try:
            with open(self.history_file, 'w', encoding='utf-8') as f:
                json.dump(list(self.processed_files), f, ensure_ascii=False, indent=2)
        except Exception as e:
            logger.error(f"保存处理记录失败: {e}")
    
    def find_audio_files(self):
        """查找音频文件"""
        audio_files = []
        
        logger.info(f"📁 扫描文件夹: {self.talk_folder}")
        
        try:
            for item in self.talk_folder.iterdir():
                if item.is_file():
                    ext = item.suffix.lower()
                    if ext in ['.mp3', '.wav', '.m4a', '.flac', '.ogg']:
                        if item.name not in self.processed_files:
                            audio_files.append(item)
        except Exception as e:
            logger.error(f"遍历文件时出错: {e}")
        
        logger.info(f"🔍 找到 {len(audio_files)} 个音频文件")
        return audio_files
    
    def transcribe_audio(self, audio_path):
        """转换音频为文本"""
        try:
            model = self.load_whisper_model()
            logger.info(f"🎵 开始转换: {audio_path.name}")
            
            result = model.transcribe(
                str(audio_path),
                language=self.language,
                fp16=False,
                temperature=0.0
            )
            
            text = result.get("text", "").strip()
            duration = result.get("duration", 0)
            
            # 文本处理流程
            if text:
                cleaned_text = self.clean_text(text)
                
                if self.convert_to_simplified:
                    simplified_text = self.traditional_to_simplified(cleaned_text)
                    logger.info(f"✅ 转换完成: {audio_path.name} (时长: {duration:.1f}s, 简体字数: {len(simplified_text)})")
                    return simplified_text, duration
                else:
                    logger.info(f"✅ 转换完成: {audio_path.name} (时长: {duration:.1f}s, 字数: {len(cleaned_text)})")
                    return cleaned_text, duration
            else:
                logger.warning(f"⚠️ 转录结果为空: {audio_path.name}")
                return "[音频内容为空或无法识别]", duration
            
        except Exception as e:
            logger.error(f"❌ 转换失败 {audio_path.name}: {e}")
            raise
    
    def save_text_file(self, audio_path, text, duration):
        """保存转录文本为纯文本"""
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            text_filename = f"{audio_path.stem}_{timestamp}.txt"
            text_path = self.text_folder / text_filename
            
            with open(text_path, 'w', encoding='utf-8') as f:
                f.write(text)
            
            logger.info(f"💾 文本已保存: {text_path.name}")
            return text_path
            
        except Exception as e:
            logger.error(f"❌ 保存文本失败: {e}")
            raise
    
    def upload_to_dify(self, text_path, audio_filename, text_content):
        """上传到Dify知识库"""
        if not self.dify_api_key or self.dify_api_key == "your-dify-api-key-here":
            logger.warning("⏭️ Dify API未配置，跳过上传")
            return False
        
        if not self.dify_kb_id or self.dify_kb_id == "your-knowledge-base-id":
            logger.warning("⏭️ 知识库ID未配置，跳过上传")
            return False
        
        try:
            import requests
            
            upload_url = f"{self.dify_base_url}/datasets/{self.dify_kb_id}/document/create-by-file"
            
            logger.info(f"📤 准备上传到Dify: {audio_filename}")
            
            headers = {
                "Authorization": f"Bearer {self.dify_api_key}",
            }
            
            process_rule = {
                "mode": "automatic",
                "rules": {
                    "pre_processing_rules": [],
                    "segmentation": {
                        "separator": "\n",
                        "max_tokens": 500
                    }
                }
            }
            
            data_json = {
                "indexing_technique": "high_quality",
                "process_rule": process_rule
            }
            
            data_json_str = json.dumps(data_json, ensure_ascii=False)
            
            with open(text_path, 'rb') as file_content:
                files = {
                    'file': (f"{audio_filename}.txt", file_content, 'text/plain')
                }
                
                form_data = {
                    'data': data_json_str
                }
                
                logger.info("🚀 开始上传文件...")
                response = requests.post(
                    upload_url,
                    headers=headers,
                    files=files,
                    data=form_data,
                    timeout=60
                )
            
            if response.status_code == 200:
                result = response.json()
                document_id = result.get('document', {}).get('id', '未知')
                logger.info(f"✅ 上传成功！文档ID: {document_id}")
                return True
            else:
                logger.error(f"❌ 上传失败 HTTP {response.status_code}")
                logger.error(f"错误详情: {response.text}")
                return False
                
        except Exception as e:
            logger.error(f"❌ Dify上传异常: {e}")
            return False
    
    def process_file(self, audio_path):
        """处理单个音频文件"""
        try:
            logger.info(f"{'='*60}")
            logger.info(f"🚀 开始处理文件: {audio_path.name}")
            logger.info(f"{'='*60}")
            
            # 1. 转录音频（包含繁简转换）
            text, duration = self.transcribe_audio(audio_path)
            
            # 2. 保存文本
            text_path = self.save_text_file(audio_path, text, duration)
            
            # 3. 上传到Dify
            upload_success = False
            if self.dify_api_key and self.dify_api_key != "your-dify-api-key-here":
                upload_success = self.upload_to_dify(text_path, audio_path.stem, text)
            else:
                logger.info("⏭️ 跳过Dify上传（未配置API）")
                upload_success = True
            
            # 4. 移动已处理文件
            processed_path = self.processed_folder / audio_path.name
            if processed_path.exists():
                timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                processed_path = self.processed_folder / f"{audio_path.stem}_{timestamp}{audio_path.suffix}"
            
            audio_path.rename(processed_path)
            logger.info(f"📦 文件已移动到: {processed_path.name}")
            
            # 5. 记录处理历史
            self.save_processed_history(audio_path.name)
            
            logger.info(f"✅ 处理完成: {audio_path.name}")
            if upload_success and self.dify_api_key != "your-dify-api-key-here":
                logger.info("✅ 已上传到Dify知识库")
            
            return True
            
        except Exception as e:
            logger.error(f"❌ 处理文件失败 {audio_path.name}: {e}")
            return False
    
    def batch_process(self):
        """批量处理模式"""
        logger.info("🔄 批量处理模式")
        
        audio_files = self.find_audio_files()
        
        if not audio_files:
            logger.info("📭 没有找到需要处理的音频文件")
            return
        
        logger.info(f"🔍 找到 {len(audio_files)} 个未处理的音频文件")
        
        success_count = 0
        for i, audio_file in enumerate(audio_files, 1):
            logger.info(f"\n📊 处理进度: [{i}/{len(audio_files)}]")
            
            if self.process_file(audio_file):
                success_count += 1
            
            if i < len(audio_files):
                time.sleep(2)
        
        logger.info(f"\n🎉 批量处理完成!")
        logger.info(f"✅ 成功: {success_count}/{len(audio_files)}")
    
    def watch_folder(self):
        """实时监控模式"""
        try:
            from watchdog.observers import Observer
            from watchdog.events import FileSystemEventHandler
        except ImportError:
            logger.error("❌ 未安装watchdog库，无法使用监控模式")
            logger.info("💡 请运行: pip install watchdog")
            return
        
        class AudioFileHandler(FileSystemEventHandler):
            def __init__(self, processor):
                self.processor = processor
                self.processing_files = set()
            
            def on_created(self, event):
                if not event.is_directory:
                    file_path = Path(event.src_path)
                    if file_path.suffix.lower() in ['.mp3', '.wav', '.m4a']:
                        if file_path.name not in self.processing_files:
                            self.processing_files.add(file_path.name)
                            logger.info(f"🎯 检测到新文件: {file_path.name}")
                            time.sleep(3)
                            try:
                                self.processor.process_file(file_path)
                            finally:
                                self.processing_files.discard(file_path.name)
        
        logger.info("👀 实时监控模式")
        logger.info(f"📁 监控文件夹: {self.talk_folder}")
        logger.info("💡 放入音频文件即可自动处理")
        logger.info("⏹️ 按 Ctrl+C 停止程序")
        
        self.load_whisper_model()
        
        event_handler = AudioFileHandler(self)
        observer = Observer()
        observer.schedule(event_handler, str(self.talk_folder), recursive=False)
        observer.start()
        
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            logger.info("\n🛑 正在停止监控...")
            observer.stop()
        
        observer.join()
        logger.info("✅ 监控已停止")

def print_banner():
    """打印程序横幅"""
    banner = """
╔══════════════════════════════════════════════════╗
║             Talk文件夹监控程序 v6.0              ║
║         MP3自动转文本 + 繁简转换 + Dify上传       ║
║             已修复繁简转换器bug                   ║
╚══════════════════════════════════════════════════╝
    """
    print(banner)

def main():
    """主程序"""
    print_banner()
    
    try:
        processor = TalkProcessor()
        
        if not processor.talk_folder.exists():
            logger.error(f"❌ 监控文件夹不存在: {processor.talk_folder}")
            return
        
        print("请选择运行模式:")
        print("1. 🔄 批量处理已有文件")
        print("2. 👀 实时监控新文件")
        print("3. 🔧 测试模式（不上传）")
        
        choice = input("\n请输入 1, 2 或 3: ").strip()
        
        if choice == '1':
            processor.batch_process()
        elif choice == '2':
            processor.watch_folder()
        elif choice == '3':
            processor.dify_api_key = ""
            processor.batch_process()
        else:
            print("❌ 无效选择，使用批量处理模式")
            processor.batch_process()
    
    except KeyboardInterrupt:
        logger.info("\n👋 程序已被用户中断")
    except Exception as e:
        logger.error(f"❌ 程序运行异常: {e}")

if __name__ == "__main__":
    main()