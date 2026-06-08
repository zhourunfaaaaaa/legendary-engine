#!/usr/bin/env node
/**
 * 独立识图脚本 — 调用智谱 GLM-4.1V Thinking Flash（免费）
 *
 * 用法:
 *   node vision.js <图片路径> [问题]
 *   node vision.js --url <图片链接> [问题]
 */

const fs = require("fs");
const path = require("path");
const https = require("https");
const http = require("http");

const BASE_URL = "https://open.bigmodel.cn/api/paas/v4";
const API_KEY = "5a0b6e83b9f44e64bc3b885e598fabca.eyDIMTNi13A2umcR";
const MODEL = "glm-4.1v-thinking-flash";

function parseArgs() {
  const argv = process.argv.slice(2);
  let imageSource = "", prompt = "", isUrl = false;

  for (let i = 0; i < argv.length; i++) {
    if (argv[i] === "--url" && argv[i + 1]) {
      isUrl = true;
      imageSource = argv[++i];
    } else if (!imageSource && !argv[i].startsWith("--")) {
      imageSource = argv[i];
    } else if (imageSource && !argv[i].startsWith("--")) {
      prompt = prompt ? prompt + " " + argv[i] : argv[i];
    }
  }
  if (!prompt) prompt = "请详细描述这张图片的内容。";
  return { imageSource, prompt, isUrl };
}

function resolveImageUrl(source, isUrl) {
  if (isUrl) return source;
  const resolved = path.resolve(source);
  if (!fs.existsSync(resolved)) throw new Error(`文件不存在: ${resolved}`);
  const ext = path.extname(resolved).toLowerCase().replace(".", "");
  const mimeMap = { jpg: "jpeg", jpeg: "jpeg", png: "png", gif: "gif", webp: "webp", bmp: "bmp" };
  const data = fs.readFileSync(resolved);

  // Check if image is too large (base64 > 2MB)
  const base64 = data.toString("base64");
  if (base64.length > 2 * 1024 * 1024) {
    throw new Error("图片文件过大，请使用小于 1.5MB 的图片。你可以用截图工具重新截取一个较小的区域。");
  }

  return `data:image/${mimeMap[ext] || "jpeg"};base64,${base64}`;
}

function request(payload) {
  const url = new URL(BASE_URL.replace(/\/?$/, "/") + "chat/completions");
  const body = JSON.stringify(payload);
  const transport = url.protocol === "https:" ? https : http;

  return new Promise((resolve, reject) => {
    const req = transport.request(url, {
      method: "POST",
      headers: {
        Authorization: `Bearer ${API_KEY}`,
        "Content-Type": "application/json",
        "Content-Length": Buffer.byteLength(body),
      },
    }, (res) => {
      let data = "";
      res.on("data", (c) => data += c);
      res.on("end", () => {
        if (res.statusCode >= 400) {
          const errObj = JSON.parse(data || "{}");
          return reject(new Error(`API ${res.statusCode}: ${errObj.error?.message || data.slice(0, 300)}`));
        }
        try {
          let content = JSON.parse(data)?.choices?.[0]?.message?.content || data;
          // Strip <think>...</think> blocks from thinking model output
          content = content.replace(/<think>[\s\S]*?<\/think>/g, "").trim();
          resolve(content || data);
        } catch { resolve(data); }
      });
    });
    req.on("error", reject);
    req.write(body);
    req.end();
  });
}

async function main() {
  const { imageSource, prompt, isUrl } = parseArgs();
  if (!imageSource) {
    console.error("用法: node vision.js <图片路径> [问题]");
    console.error("      node vision.js --url <图片链接> [问题]");
    process.exit(1);
  }
  try {
    const imageUrl = resolveImageUrl(imageSource, isUrl);
    const base64Len = imageUrl.indexOf("base64,") !== -1 ? imageUrl.split("base64,")[1].length : 0;
    console.error(`[vision] 图片大小: ${(base64Len / 1024).toFixed(0)}KB, 模型: ${MODEL}`);

    const result = await request({
      model: MODEL,
      messages: [{ role: "user", content: [
        { type: "image_url", image_url: { url: imageUrl } },
        { type: "text", text: prompt },
      ]}],
      stream: false,
      max_tokens: 4096,
    });
    console.log(result);
  } catch (err) {
    console.error("识图失败:", err.message);
    process.exit(1);
  }
}

main();
