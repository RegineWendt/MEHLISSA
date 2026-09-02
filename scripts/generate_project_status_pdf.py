# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Render the maintained MEHLISSA project-status Markdown as a shareable PDF.

Run from the repository root with a Python environment that provides ReportLab:

    python scripts/generate_project_status_pdf.py
"""

from __future__ import annotations

import html
import re
from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.enums import TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import mm
from reportlab.platypus import (
    BaseDocTemplate,
    Flowable,
    Frame,
    PageBreak,
    PageTemplate,
    Paragraph,
    Spacer,
    Table,
    TableStyle,
)


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "docs" / "PROJECT_STATUS_AND_COLLABORATION_BRIEF.md"
OUTPUT = ROOT / "output" / "pdf" / "MEHLISSA_Next_Project_Status_and_Collaboration_Brief.pdf"

NAVY = colors.HexColor("#163B57")
DEEP_NAVY = colors.HexColor("#0E293D")
TEAL = colors.HexColor("#159C95")
PALE_TEAL = colors.HexColor("#E8F5F3")
BLUE = colors.HexColor("#377CA3")
ORANGE = colors.HexColor("#E28A35")
INK = colors.HexColor("#26343D")
MUTED = colors.HexColor("#647581")
RULE = colors.HexColor("#CCD7DD")
PALE_GREY = colors.HexColor("#F4F7F8")
WHITE = colors.white
FONT = "Helvetica"
FONT_BOLD = "Helvetica-Bold"


class AccentRule(Flowable):
    def __init__(self, width: float, color=TEAL, thickness: float = 2):
        super().__init__()
        self.width = width
        self.height = thickness
        self.color = color
        self.thickness = thickness

    def draw(self):
        self.canv.setStrokeColor(self.color)
        self.canv.setLineWidth(self.thickness)
        self.canv.line(0, 0, self.width, 0)


class StatusDocument(BaseDocTemplate):
    def __init__(self, filename: str, **kwargs):
        super().__init__(filename, **kwargs)
        body_frame = Frame(
            self.leftMargin,
            self.bottomMargin,
            self.width,
            self.height,
            leftPadding=0,
            rightPadding=0,
            topPadding=0,
            bottomPadding=0,
            id="body",
        )
        cover_frame = Frame(0, 0, A4[0], A4[1], 0, 0, 0, 0, id="cover")
        self.addPageTemplates(
            [
                PageTemplate(id="cover", frames=[cover_frame], onPage=self.draw_cover),
                PageTemplate(id="body", frames=[body_frame], onPage=self.draw_body),
            ]
        )

    def afterPage(self):
        if self.page == 1:
            self.handle_nextPageTemplate("body")

    def draw_cover(self, canvas, _doc):
        width, height = A4
        canvas.saveState()
        canvas.setFillColor(DEEP_NAVY)
        canvas.rect(0, 0, width, height, fill=1, stroke=0)
        canvas.setFillColor(TEAL)
        canvas.rect(0, height - 13 * mm, width, 13 * mm, fill=1, stroke=0)
        canvas.setFillColor(BLUE)
        canvas.circle(width - 33 * mm, 34 * mm, 61 * mm, fill=1, stroke=0)
        canvas.setFillColor(colors.Color(1, 1, 1, alpha=0.08))
        canvas.circle(width - 18 * mm, 54 * mm, 43 * mm, fill=1, stroke=0)
        canvas.restoreState()

    def draw_body(self, canvas, doc):
        width, height = A4
        canvas.saveState()
        canvas.setStrokeColor(RULE)
        canvas.setLineWidth(0.5)
        canvas.line(doc.leftMargin, height - 16 * mm, width - doc.rightMargin, height - 16 * mm)
        canvas.setFont(FONT_BOLD, 8.5)
        canvas.setFillColor(NAVY)
        canvas.drawString(doc.leftMargin, height - 12 * mm, "MEHLISSA NEXT")
        canvas.setFont(FONT, 8.5)
        canvas.setFillColor(MUTED)
        canvas.drawRightString(width - doc.rightMargin, height - 12 * mm, "Project Status and Collaboration Brief")
        canvas.line(doc.leftMargin, 14 * mm, width - doc.rightMargin, 14 * mm)
        canvas.setFont(FONT, 8)
        canvas.drawString(doc.leftMargin, 9.5 * mm, "Research software demonstrator - not for clinical decision-making")
        canvas.drawRightString(width - doc.rightMargin, 9.5 * mm, str(doc.page))
        canvas.restoreState()


def make_styles():
    result = getSampleStyleSheet()
    result.add(ParagraphStyle(name="CoverKicker", fontName=FONT_BOLD, fontSize=11, leading=14, textColor=colors.HexColor("#7FE0D8"), spaceAfter=7 * mm))
    result.add(ParagraphStyle(name="CoverTitle", fontName=FONT_BOLD, fontSize=31, leading=34, textColor=WHITE, spaceAfter=7 * mm))
    result.add(ParagraphStyle(name="CoverSubtitle", fontName=FONT, fontSize=15, leading=21, textColor=colors.HexColor("#D9E7EF"), spaceAfter=13 * mm))
    result.add(ParagraphStyle(name="CoverMeta", fontName=FONT, fontSize=9.5, leading=15, textColor=colors.HexColor("#C2D3DD")))
    result.add(ParagraphStyle(name="H1x", fontName=FONT_BOLD, fontSize=19, leading=23, textColor=NAVY, spaceAfter=4 * mm, keepWithNext=True))
    result.add(ParagraphStyle(name="H2x", fontName=FONT_BOLD, fontSize=13.5, leading=17, textColor=NAVY, spaceBefore=4 * mm, spaceAfter=2 * mm, keepWithNext=True))
    result.add(ParagraphStyle(name="BodyX", fontName=FONT, fontSize=9.1, leading=13, textColor=INK, spaceAfter=2.2 * mm))
    result.add(ParagraphStyle(name="BulletX", fontName=FONT, fontSize=8.9, leading=12.4, textColor=INK, leftIndent=3.2 * mm, firstLineIndent=0, spaceAfter=1.1 * mm))
    result.add(ParagraphStyle(name="BulletCell", fontName=FONT, fontSize=8.9, leading=12.4, textColor=INK))
    result.add(ParagraphStyle(name="TableHead", fontName=FONT_BOLD, fontSize=7.8, leading=10.1, textColor=WHITE, alignment=TA_LEFT))
    result.add(ParagraphStyle(name="TableCell", fontName=FONT, fontSize=7.35, leading=9.8, textColor=INK))
    result.add(ParagraphStyle(name="TableCellBold", fontName=FONT_BOLD, fontSize=7.45, leading=9.8, textColor=NAVY))
    result.add(ParagraphStyle(name="CodeX", fontName="Courier", fontSize=7.4, leading=10.4, textColor=DEEP_NAVY, leftIndent=4 * mm, rightIndent=4 * mm, borderColor=RULE, borderWidth=0.6, borderPadding=5, backColor=PALE_GREY, spaceBefore=2 * mm, spaceAfter=3 * mm))
    return result


STYLES = make_styles()


def rich_text(value: str) -> str:
    escaped = html.escape(value.strip())
    escaped = re.sub(r"`([^`]+)`", r'<font name="Courier">\1</font>', escaped)
    escaped = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", escaped)
    return escaped.replace("  ", " ")


def paragraph(value: str, style: str = "BodyX") -> Paragraph:
    return Paragraph(rich_text(value), STYLES[style])


def list_item(marker: str, value: str) -> Table:
    result = Table(
        [[Paragraph(html.escape(marker), STYLES["BulletCell"]), Paragraph(rich_text(value), STYLES["BulletCell"])]],
        colWidths=[5 * mm, 158 * mm],
        hAlign="LEFT",
    )
    result.setStyle(
        TableStyle(
            [
                ("VALIGN", (0, 0), (-1, -1), "TOP"),
                ("LEFTPADDING", (0, 0), (-1, -1), 0),
                ("RIGHTPADDING", (0, 0), (-1, -1), 1),
                ("TOPPADDING", (0, 0), (-1, -1), 0),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 3),
            ]
        )
    )
    return result


def markdown_table(rows: list[list[str]]) -> Table:
    rows = [row for row in rows if not all(re.fullmatch(r":?-{3,}:?", cell.strip()) for cell in row)]
    columns = len(rows[0])
    if columns == 2:
        widths = [53 * mm, 113 * mm]
    elif rows[0][0].strip().lower() in {"gate", "package", "area", "scope"}:
        widths = [28 * mm, 63 * mm, 75 * mm]
    else:
        widths = [35 * mm, 70 * mm, 61 * mm]
    cooked = []
    for row_index, row in enumerate(rows):
        cooked_row = []
        for column_index, cell in enumerate(row):
            style = "TableHead" if row_index == 0 else ("TableCellBold" if column_index == 0 else "TableCell")
            cooked_row.append(paragraph(cell, style))
        cooked.append(cooked_row)
    result = Table(cooked, colWidths=widths, repeatRows=1, hAlign="LEFT")
    result.setStyle(
        TableStyle(
            [
                ("VALIGN", (0, 0), (-1, -1), "TOP"),
                ("LEFTPADDING", (0, 0), (-1, -1), 5),
                ("RIGHTPADDING", (0, 0), (-1, -1), 5),
                ("TOPPADDING", (0, 0), (-1, -1), 5),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 5),
                ("GRID", (0, 0), (-1, -1), 0.35, RULE),
                ("BACKGROUND", (0, 0), (-1, 0), NAVY),
                ("ROWBACKGROUNDS", (0, 1), (-1, -1), [WHITE, PALE_GREY]),
            ]
        )
    )
    return result


def flush_paragraph(buffer: list[str], story: list):
    if buffer:
        story.append(paragraph(" ".join(part.strip() for part in buffer)))
        buffer.clear()


def parse_body(markdown: str) -> list:
    lines = markdown.splitlines()
    start = next(index for index, line in enumerate(lines) if line.strip() == "## Executive summary")
    story: list = []
    paragraph_buffer: list[str] = []
    table_rows: list[list[str]] = []
    code_lines: list[str] = []
    in_code = False
    first_section = True

    def flush_table():
        if table_rows:
            story.append(markdown_table(table_rows.copy()))
            story.append(Spacer(1, 2.5 * mm))
            table_rows.clear()

    for raw in lines[start:]:
        line = raw.rstrip()
        stripped = line.strip()
        if stripped.startswith("```"):
            flush_paragraph(paragraph_buffer, story)
            flush_table()
            if in_code:
                story.append(Paragraph("<br/>".join(html.escape(item) for item in code_lines), STYLES["CodeX"]))
                code_lines.clear()
            in_code = not in_code
            continue
        if in_code:
            code_lines.append(line)
            continue
        if stripped.startswith("|") and stripped.endswith("|"):
            flush_paragraph(paragraph_buffer, story)
            table_rows.append([cell.strip() for cell in stripped.strip("|").split("|")])
            continue
        flush_table()
        if stripped.startswith("## "):
            flush_paragraph(paragraph_buffer, story)
            if not first_section:
                story.append(PageBreak())
            first_section = False
            story.append(paragraph(stripped[3:], "H1x"))
            story.append(AccentRule(32 * mm))
            story.append(Spacer(1, 3 * mm))
        elif stripped.startswith("### "):
            flush_paragraph(paragraph_buffer, story)
            story.append(paragraph(stripped[4:], "H2x"))
        elif re.match(r"^\d+\. ", stripped):
            flush_paragraph(paragraph_buffer, story)
            marker, value = stripped.split(" ", 1)
            story.append(list_item(marker, value))
        elif stripped.startswith("- "):
            flush_paragraph(paragraph_buffer, story)
            story.append(list_item("-", stripped[2:]))
        elif not stripped:
            flush_paragraph(paragraph_buffer, story)
        elif stripped.startswith("<!--") or stripped.startswith("-->"):
            continue
        else:
            paragraph_buffer.append(stripped)
    flush_paragraph(paragraph_buffer, story)
    flush_table()
    return story


def cover_story() -> list:
    status = Table(
        [
            [paragraph("M0-M7", "TableHead"), paragraph("PASSED", "TableHead"), paragraph("CURRENT FOCUS", "TableHead")],
            [Paragraph("Architecture and implementation", STYLES["CoverMeta"]), Paragraph("All milestone gates", STYLES["CoverMeta"]), Paragraph("UX-5 Python API and notebooks", STYLES["CoverMeta"])],
        ],
        colWidths=[51 * mm, 37 * mm, 76 * mm],
        style=TableStyle(
            [
                ("BACKGROUND", (0, 0), (-1, 0), TEAL),
                ("BACKGROUND", (0, 1), (-1, 1), colors.Color(1, 1, 1, alpha=0.12)),
                ("BOX", (0, 0), (-1, -1), 0.6, colors.HexColor("#80B7C1")),
                ("INNERGRID", (0, 0), (-1, -1), 0.3, colors.HexColor("#80B7C1")),
                ("VALIGN", (0, 0), (-1, -1), "TOP"),
                ("LEFTPADDING", (0, 0), (-1, -1), 7),
                ("RIGHTPADDING", (0, 0), (-1, -1), 7),
                ("TOPPADDING", (0, 0), (-1, -1), 7),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 7),
            ]
        ),
    )
    return [
        Spacer(1, 40 * mm),
        Paragraph("PROJECT STATUS / COLLABORATION BRIEF", STYLES["CoverKicker"]),
        Paragraph("MEHLISSA Next", STYLES["CoverTitle"]),
        Paragraph("A reproducible multilayer research platform for body transport, molecular interaction, cell response, and Nano-IoT communication", STYLES["CoverSubtitle"]),
        status,
        Spacer(1, 17 * mm),
        Paragraph("Prepared for prospective contributors and research partners", STYLES["CoverMeta"]),
        Paragraph("Status date: 2 September 2026", STYLES["CoverMeta"]),
        Paragraph("Branch: mehlissa-next-generation | Base revision: f3739e4", STYLES["CoverMeta"]),
        Paragraph("Repository: github.com/RegineWendt/MEHLISSA", STYLES["CoverMeta"]),
        PageBreak(),
    ]


def main():
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    markdown = SOURCE.read_text(encoding="utf-8")
    story = cover_story() + parse_body(markdown)
    document = StatusDocument(
        str(OUTPUT),
        pagesize=A4,
        rightMargin=22 * mm,
        leftMargin=22 * mm,
        topMargin=23 * mm,
        bottomMargin=20 * mm,
        title="MEHLISSA Next - Project Status and Collaboration Brief",
        author="MEHLISSA contributors",
        subject="Project status through M7, current uses, limitations, and next steps",
    )
    document.build(story)
    print(OUTPUT)


if __name__ == "__main__":
    main()
