"""Generate full academic mini project report - SMART_GLASSES_REPORT.docx"""
import sys, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

from docx import Document
from docx.shared import Pt, Inches
from report_helpers import *
from report_part_a import *
from report_part_b import *
from report_part_c import *

def generate():
    doc = Document()
    style = doc.styles['Normal']
    style.font.name = 'Times New Roman'
    style.font.size = Pt(12)
    style.paragraph_format.space_before = Pt(0)
    style.paragraph_format.space_after = Pt(0)

    for s in doc.sections:
        s.top_margin = Inches(1)
        s.bottom_margin = Inches(1)
        s.left_margin = Inches(1.25)
        s.right_margin = Inches(1)

    make_title_page(doc)
    write_abstract(doc, add_heading1, add_heading2, add_body, add_bullet)
    write_acknowledgement(doc, add_heading1, add_body, add_centered)
    write_toc(doc, add_heading1, add_body)
    write_ch1(doc, add_heading1, add_heading2, add_body, add_bullet)
    write_ch2(doc, add_heading1, add_heading2, add_body, add_bullet)
    write_ch3(doc, add_heading1, add_heading2, add_body, add_bullet)
    write_ch4(doc, add_heading1, add_heading2, add_heading3, add_body, add_bullet)
    write_ch5(doc, add_heading1, add_heading2, add_heading3, add_body, add_bullet)
    write_ch6(doc, add_heading1, add_heading2, add_heading3, add_body, add_bullet)
    write_ch7(doc, add_heading1, add_heading2, add_body, add_bullet)
    write_ch8(doc, add_heading1, add_heading2, add_body, add_bullet)
    write_ch9(doc, add_heading1, add_body)
    write_ch10(doc, add_heading1, add_body, add_bullet)

    out = r'c:\Users\harin\Desktop\smart Glasses\SMART_GLASSES_REPORT.docx'
    doc.save(out)
    print(f'Report generated: {out}')
    print(f'Total pages (approx): {doc.element.body.countchildren()}+ elements')

if __name__ == '__main__':
    generate()
