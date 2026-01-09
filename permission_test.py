import os
import requests
from dotenv import load_dotenv

load_dotenv()  # 加载你的.env文件

def comprehensive_permission_check():
    """全面的权限检查"""
    api_key = os.getenv("DIFY_API_KEY")
    base_url = os.getenv("DIFY_API_URL", "https://api.dify.ai/v1")
    kb_id = os.getenv("DIFY_KB_ID")
    
    headers = {"Authorization": f"Bearer {api_key}"}
    
    tests = [
        {
            "name": "基础API连通性",
            "url": f"{base_url}/health",
            "method": "GET"
        },
        {
            "name": "知识库列表访问",
            "url": f"{base_url}/datasets",
            "method": "GET"
        },
        {
            "name": "特定知识库详情",
            "url": f"{base_url}/knowledge_bases/{kb_id}",
            "method": "GET"
        },
        {
            "name": "应用列表（如果有对话型应用）",
            "url": f"{base_url}/app",
            "method": "GET"
        }
    ]
    
    print("🔍 开始全面权限检查...")
    print("=" * 50)
    
    for test in tests:
        try:
            if test["method"] == "GET":
                response = requests.get(test["url"], headers=headers, timeout=10)
            else:
                response = requests.post(test["url"], headers=headers, timeout=10)
            
            print(f"\n📊 {test['name']}:")
            print(f"   状态码: {response.status_code}")
            
            if response.status_code == 200:
                print("   ✅ 权限正常")
            elif response.status_code == 401:
                print("   ❌ 认证失败 - API密钥无效")
            elif response.status_code == 403:
                print("   ❌ 权限不足 - 无访问权限")
            elif response.status_code == 404:
                print("   ⚠️  资源不存在 - 检查URL或ID")
            else:
                print(f"   ❓ 其他错误: {response.text[:100]}")
                
        except Exception as e:
            print(f"   💥 请求异常: {e}")
    
    print("=" * 50)
    print("🔚 检查完成")

# 3. 运行测试
if __name__ == "__main__":
    comprehensive_permission_check()