TEMPLATE = subdirs

src_pdf.subdir = pdf
src_pdf.depends = lib

SUBDIRS = lib src_pdf

qtHaveModule(widgets) {
    src_pdfwidgets.subdir = pdfwidgets
    src_pdfwidgets.depends = src_pdf

    SUBDIRS += src_pdfwidgets
}
