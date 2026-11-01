import os
import zipfile
import zlib

fixtures_dir = os.path.join(os.path.dirname(__file__), 'test_fixtures')
os.makedirs(fixtures_dir, exist_ok=True)

# 1. DOCX
docx_path = os.path.join(fixtures_dir, 'sample.docx')
with zipfile.ZipFile(docx_path, 'w', zipfile.ZIP_DEFLATED) as zf:
    doc_xml = '<?xml version="1.0" encoding="UTF-8"?><w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:body><w:p><w:r><w:t>IndexIt Docx Extraction Test Phrase</w:t></w:r></w:p></w:body></w:document>'
    zf.writestr('word/document.xml', doc_xml)
print('Created sample.docx')

# 2. PPTX
pptx_path = os.path.join(fixtures_dir, 'sample.pptx')
with zipfile.ZipFile(pptx_path, 'w', zipfile.ZIP_DEFLATED) as zf:
    slide_xml = '<?xml version="1.0" encoding="UTF-8"?><p:sld xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main"><p:cSld><p:spTree><p:sp><p:txBody><p:p><p:r><p:t>IndexIt Presentation Slide Test Phrase</p:t></p:r></p:p></p:txBody></p:sp></p:spTree></p:cSld></p:sld>'
    zf.writestr('ppt/slides/slide1.xml', slide_xml)
print('Created sample.pptx')

# 3. XLSX
xlsx_path = os.path.join(fixtures_dir, 'sample.xlsx')
with zipfile.ZipFile(xlsx_path, 'w', zipfile.ZIP_DEFLATED) as zf:
    shared_xml = '<?xml version="1.0" encoding="UTF-8"?><sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"><si><t>IndexIt Spreadsheet Cell Test Phrase</t></si></sst>'
    zf.writestr('xl/sharedStrings.xml', shared_xml)
print('Created sample.xlsx')

# 4. ODT
odt_path = os.path.join(fixtures_dir, 'sample.odt')
with zipfile.ZipFile(odt_path, 'w', zipfile.ZIP_DEFLATED) as zf:
    content_xml = '<?xml version="1.0" encoding="UTF-8"?><office:document-content xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"><office:body><office:text><text:p>IndexIt OpenDocument Text Test Phrase</text:p></office:text></office:body></office:document-content>'
    zf.writestr('content.xml', content_xml)
print('Created sample.odt')

# 5. ZIP
zip_path = os.path.join(fixtures_dir, 'sample.zip')
with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zf:
    zf.writestr('nested_note.txt', 'IndexIt Archive Compressed Test Phrase inside zip container')
print('Created sample.zip')

# 6. RTF
rtf_path = os.path.join(fixtures_dir, 'sample.rtf')
with open(rtf_path, 'w', encoding='ascii') as f:
    f.write(r'{\rtf1\ansi\deff0 {\fonttbl{\f0 Times New Roman;}} \f0\fs24 IndexIt RichText Document Test Phrase \par}')
print('Created sample.rtf')

# 7. EML
eml_path = os.path.join(fixtures_dir, 'sample.eml')
with open(eml_path, 'w', encoding='utf-8') as f:
    f.write('From: sender@example.com\nTo: recipient@example.com\nSubject: Project Update\nDate: Mon, 29 Oct 2026 12:00:00 +0000\n\nHello Team, IndexIt Email Message Test Phrase here.\n')
print('Created sample.eml')

# 8. HTML
html_path = os.path.join(fixtures_dir, 'sample.html')
with open(html_path, 'w', encoding='utf-8') as f:
    f.write('<!DOCTYPE html><html><head><title>Test</title><script>var ignore = 1;</script></head><body><h1>Heading</h1><p>IndexIt HyperText Markup Test Phrase</p></body></html>')
print('Created sample.html')

# 9. PDF
pdf_path = os.path.join(fixtures_dir, 'sample.pdf')
stream_data = b'BT /F1 12 Tf 72 712 Td (IndexIt Portable Document Test Phrase) Tj ET'
compressed_stream = zlib.compress(stream_data)

pdf_content = (
    b'%PDF-1.4\n'
    b'1 0 obj << /Type /Catalog /Pages 2 0 R >> endobj\n'
    b'2 0 obj << /Type /Pages /Kids [3 0 R] /Count 1 >> endobj\n'
    b'3 0 obj << /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Contents 4 0 R >> endobj\n'
    b'4 0 obj << /Length ' + str(len(compressed_stream)).encode() + b' /Filter /FlateDecode >>\n'
    b'stream\n' + compressed_stream + b'\nendstream\nendobj\n'
    b'xref\n0 5\n0000000000 65535 f \n'
    b'trailer << /Size 5 /Root 1 0 R >>\nstartxref\n999\n%%EOF\n'
)
with open(pdf_path, 'wb') as f:
    f.write(pdf_content)
print('Created sample.pdf')
