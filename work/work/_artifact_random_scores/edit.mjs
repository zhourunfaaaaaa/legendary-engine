import fs from "node:fs/promises";
import { randomInt } from "node:crypto";
import { FileBlob, SpreadsheetFile } from "@oai/artifact-tool";

const inputPath = "C:/Users/qq137/Downloads/网安251班综合素质测评民主评议打分表 .xlsx";
const outputDir = "D:/github/legendary-engine/work/outputs/01a05d30-5fb3-7fb0-a30e-20071a193a77";
const outputPath = `${outputDir}/网安251班综合素质测评民主评议打分表_已填写.xlsx`;
const previewDir = `${outputDir}/verification-previews`;

const input = await FileBlob.load(inputPath);
const workbook = await SpreadsheetFile.importXlsx(input);
const scoreSheet = workbook.worksheets.getItem("民主评议打分表");

const scores = Array.from({ length: 51 }, () =>
  Array.from({ length: 3 }, () => randomInt(160, 201) / 10),
);
scoreSheet.getRange("D6:F56").values = scores;

const flatScores = scores.flat();
if (flatScores.length !== 153 || flatScores.some((score) => score < 16 || score > 20)) {
  throw new Error("Generated score validation failed.");
}

const scoreCheck = await workbook.inspect({
  kind: "table",
  range: "民主评议打分表!A1:F56",
  include: "values,formulas",
  tableMaxRows: 56,
  tableMaxCols: 6,
  maxChars: 20000,
});
console.log(scoreCheck.ndjson);

const errors = await workbook.inspect({
  kind: "match",
  searchTerm: "#REF!|#DIV/0!|#VALUE!|#NAME\\?|#N/A",
  options: { useRegex: true, maxResults: 300 },
  summary: "final formula error scan",
});
console.log(errors.ndjson);
console.log(JSON.stringify({
  scoreCount: flatScores.length,
  minimum: Math.min(...flatScores),
  maximum: Math.max(...flatScores),
  topValues: scoreSheet.getRange("A3:F4").values,
  topFormulas: scoreSheet.getRange("A3:F4").formulas,
}));

await fs.mkdir(previewDir, { recursive: true });
for (let i = 0; i < workbook.worksheets.items.length; i += 1) {
  const sheet = workbook.worksheets.getItemAt(i);
  const preview = await workbook.render({ sheetName: sheet.name, autoCrop: "all", scale: 1.5, format: "png" });
  await fs.writeFile(`${previewDir}/sheet-${i + 1}.png`, new Uint8Array(await preview.arrayBuffer()));
}

await fs.mkdir(outputDir, { recursive: true });
const output = await SpreadsheetFile.exportXlsx(workbook);
await output.save(outputPath);
console.log(`OUTPUT ${outputPath}`);
