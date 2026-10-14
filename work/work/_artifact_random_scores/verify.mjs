import { FileBlob, SpreadsheetFile } from "@oai/artifact-tool";

const outputPath = "D:/github/legendary-engine/work/outputs/01a05d30-5fb3-7fb0-a30e-20071a193a77/网安251班综合素质测评民主评议打分表_已填写.xlsx";
const workbook = await SpreadsheetFile.importXlsx(await FileBlob.load(outputPath));
const sheet = workbook.worksheets.getItem("民主评议打分表");
const values = sheet.getRange("D6:F56").values.flat();
const formulas = sheet.getRange("A3:F4").formulas;
const topValues = sheet.getRange("A3:F4").values;
const invalid = values.filter((value) => typeof value !== "number" || value < 16 || value > 20);
const blanks = values.filter((value) => value === null || value === "").length;
const errors = await workbook.inspect({
  kind: "match",
  searchTerm: "#REF!|#DIV/0!|#VALUE!|#NAME\\?|#N/A",
  options: { useRegex: true, maxResults: 300 },
  summary: "exported workbook formula error scan",
});

const result = {
  sheetNames: workbook.worksheets.items.map((item) => item.name),
  scoreCount: values.length,
  blankCount: blanks,
  invalidCount: invalid.length,
  minimum: Math.min(...values),
  maximum: Math.max(...values),
  filledCount: topValues[0][5],
  status: topValues[1][0],
  formulas,
  errorScan: errors.ndjson,
};
console.log(JSON.stringify(result, null, 2));

if (
  result.scoreCount !== 153 ||
  result.blankCount !== 0 ||
  result.invalidCount !== 0 ||
  result.filledCount !== 153 ||
  result.status !== "填写完成，可以提交" ||
  !result.errorScan.includes("matched 0 entries")
) {
  throw new Error("Exported workbook verification failed.");
}
