#!/usr/bin/env python3

# hardware_para_generator.py
import argparse
import sys
import re
import os
from pathlib import Path
from datetime import datetime

def validate_python_version():
    if sys.version_info < (3, 6):
        sys.stderr.write("Error: Requires Python 3.6 or higher\n")
        sys.exit(1)

def main():
    validate_python_version()
    
    # 配置固定路径
    script_dir = Path(__file__).parent.resolve()
    template_path = script_dir / "hardware_wukong100.para.template"
    output_path = script_dir.parents[4] / "out/wukong100/obj/vendor/revoview/wukong100/etc/param/hardware_wukong100.para"

    # 参数解析（修改为可选参数）
    parser = argparse.ArgumentParser(description="Hardware Parameter Generator")
    parser.add_argument("--param", 
                      action="append", 
                      required=False,  # 改为非必需
                      default=[],
                      help="Parameters in KEY=VALUE format")
    args = parser.parse_args()

    # 参数处理
    params = {}
    for param in args.param:
        try:
            key, value = param.split("=", 1)
            params[key.strip()] = value.strip()
        except ValueError:
            sys.stderr.write(f"Error: Invalid parameter format '{param}'. Expected KEY=VALUE\n")
            sys.exit(1)

    # 强制注入硬件版本日期及时间
    current_date = datetime.now().strftime("%y.%m.%d")
    current_date = datetime.now().strftime("%Y%m%d_%H%M")
    params["HARDWARE_VERSION"] = current_date

    # 模板处理
    try:
        with open(template_path, "r") as f:
            content = f.read()
    except FileNotFoundError:
        sys.stderr.write(f"Fatal Error: Missing template {template_path}\n")
        sys.exit(1)
    except Exception as e:
        sys.stderr.write(f"Template Error: {str(e)}\n")
        sys.exit(1)

    # 变量替换（增强容错处理）
    missing_params = set()
    def replace_handler(match):
        key = match.group(1).strip()
        if key in params:
            return params[key]
        missing_params.add(key)
        return match.group(0)  # 保留原未替换内容

    pattern = re.compile(
        r"\{\{\s*([A-Za-z0-9_]+)\s*\}\}",  # 更宽松的正则匹配
        flags=re.IGNORECASE
    )
    
    processed_content = pattern.sub(replace_handler, content)

    # 处理缺失参数警告
    if missing_params:
        sys.stderr.write(f"Warning: Missing parameters detected - {', '.join(missing_params)}\n")
        sys.stderr.write("These placeholders will remain unmodified in the output file.\n")

    # 输出处理
    try:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        with open(output_path, "w") as f:
            f.write(processed_content)
        print(f"Configuration generated: {output_path}")
        print(f"Auto-injected HARDWARE_VERSION: {current_date}")
    except PermissionError:
        sys.stderr.write(f"Permission Denied: Cannot write to {output_path}\n")
        sys.exit(1)
    except Exception as e:
        sys.stderr.write(f"Output Error: {str(e)}\n")
        sys.exit(1)

if __name__ == "__main__":
    main()