#!/usr/bin/env python3
import sys
import json

def main():
    data = json.load(sys.stdin)
    for asset in data.get("assets", []):
        name = asset.get("name", "")
        if "ucrt" in name and "ubuntu" in name and "x86_64" in name and name.endswith(".tar.xz"):
            print(asset["browser_download_url"])
            return

if __name__ == "__main__":
    main()
