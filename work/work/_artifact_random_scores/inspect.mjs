import fs from "node:fs/promises";
import { FileBlob, SpreadsheetFile } from "@oai/artifact-tool";

const inputPath = "C:/Users/qq137/Downloads/网安251班综合素质测评民主评议打分表 .xlsx";
const previewDir = "D:/github/legendary-engine/work/work/_artifact_random_scores/previews-before";

await fs.mkdir(previewDir, { recursive: true });
const input = await FileBlob.load(inputPath);
const workbook = await SpreadsheetFile.importXlsx(input);

const summary = await workbook.inspect({
  kind: "workbook,sheet,table",
  maxChars: 12000,
  tableMaxRows: 20,
  tableMaxCols: 20,
  tableMaxCellChars: 100,
});
console.log(summary.ndjson);

const sheetSummary = await workbook.inspect({ kind: "sheet", include: "id,name", maxChars: 5000 });
console.log(sheetSummary.ndjson);

const formulaSummary = await workbook.inspect({
  kind: "formula",
  sheetId: "民主评议打分表",
  range: "A1:F56",
  maxChars: 6000,
  options: { maxResults: 100 },
});
console.log(formulaSummary.ndjson);
console.log("TOP_FORMULAS", JSON.stringify(workbook.worksheets.getItem("民主评议打分表").getRange("A1:F5").formulas));

for (let i = 0; i < workbook.worksheets.items.length; i += 1) {
  const sheet = workbook.worksheets.getItemAt(i);
  const used = sheet.getUsedRange();
  console.log(`SHEET ${i}: ${sheet.name}`);
  if (used) {
    console.log(`USED ${used.address}`);
    const region = await workbook.inspect({
      kind: "region",
      sheetId: sheet.name,
      range: used.address,
      maxChars: 16000,
      tableMaxRows: 80,
      tableMaxCols: 40,
      tableMaxCellChars: 80,
    });
    console.log(region.ndjson);
    const styles = await workbook.inspect({
      kind: "computedStyle",
      sheetId: sheet.name,
      range: used.address,
      maxChars: 6000,
    });
    console.log(styles.ndjson);
  }
  const preview = await workbook.render({ sheetName: sheet.name, autoCrop: "all", scale: 1.5, format: "png" });
  await fs.writeFile(`${previewDir}/sheet-${i + 1}.png`, new Uint8Array(await preview.arrayBuffer()));
}
