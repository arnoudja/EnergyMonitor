
#ifndef DISPLAYTABLE_H
#define	DISPLAYTABLE_H

#include <cairo/cairo.h>

#include <string>
#include <vector>

class DisplayTable
{
public:
    DisplayTable()                          {}
    virtual ~DisplayTable()                 {}

    void setText(unsigned int col, unsigned int row, unsigned int line, const std::string& text, bool rightAligned = false);

    void drawTable(cairo_t* surface, unsigned int x, unsigned int y) const;

private:
    size_t nrRows() const;
    size_t nrCols() const;

    size_t getColWidth(cairo_t* surface, unsigned int col) const;
    size_t getRowLines(unsigned int row) const;

    static size_t getTextWidth(cairo_t* surface, const std::string& text);
    
    static void line(cairo_t* surface, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
    static void writeText(cairo_t* surface, unsigned int x, unsigned int y, const std::string& text, bool rightAligned = false);

private:
    struct TTextData
    {
        std::string myText;
        bool        myRightAligned;
    };

    typedef std::vector<TTextData>  TCellData;
    typedef std::vector<TCellData>  TRowData;
    typedef std::vector<TRowData>   TTableData;

    TTableData  myTableData;
};

#endif	/* DISPLAYTABLE_H */
