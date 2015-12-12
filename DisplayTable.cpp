
#include "DisplayTable.h"

#include <iostream>

using namespace std;

namespace
{
    const int cHorizontalTextOffset = 5;
    const int cVerticalTextOffset   = 3;
    const int cTextHeight           = 12;
}

void DisplayTable::setText(unsigned int col, unsigned int row, unsigned int line, const string& text, bool rightAligned)
{
    if (myTableData.size() < row + 1)
    {
        myTableData.resize(row + 1);
    }

    if (myTableData[row].size() < col + 1)
    {
        myTableData[row].resize(col + 1);
    }

    if (myTableData[row][col].size() < line + 1)
    {
        myTableData[row][col].resize(line + 1);
    }

    myTableData[row][col][line].myText         = text;
    myTableData[row][col][line].myRightAligned = rightAligned;
}

void DisplayTable::drawTable(cairo_t* surface, unsigned int x, unsigned int y) const
{
    cairo_set_source_rgb(surface, 255, 255, 255);
    cairo_select_font_face(surface, "Fixed", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(surface, 12);

    size_t rows = nrRows();
    size_t cols = nrCols();

    size_t tableHeight = 0;
    vector<size_t> rowLines(rows + 1);
    vector<size_t> rowStartPos(rows + 1);

    for (size_t i = 0; i < rows; ++i)
    {
        rowStartPos[i] = tableHeight + y;
        rowLines[i] = getRowLines(i);
        tableHeight += (rowLines[i] * cTextHeight + cVerticalTextOffset);
    }
    rowStartPos[rows] = tableHeight + y;

    size_t tableWidth = 0;
    vector<size_t> colStartPos(cols + 1);

    for (size_t i = 0; i < cols; ++i)
    {
        colStartPos[i] = tableWidth + x;
        tableWidth += getColWidth(surface, i);
    }
    colStartPos[cols] = tableWidth + x;

    for (size_t i = 0; i <= rows; ++i)
    {
        line(surface, x, rowStartPos[i], x + tableWidth, rowStartPos[i]);
    }

    for (size_t i = 0; i <= cols; ++i)
    {
        line(surface, colStartPos[i], y, colStartPos[i], y + tableHeight);
    }

    for (size_t row = 0; row < rows; ++row)
    {
        for (size_t col = 0; col < myTableData[row].size(); ++col)
        {
            TCellData cellData = myTableData[row][col];

            for (size_t line = 0; line < cellData.size(); ++line)
            {
                writeText(surface,
                          cellData[line].myRightAligned ? colStartPos[col + 1] - cHorizontalTextOffset : colStartPos[col] + cHorizontalTextOffset,
                          rowStartPos[row] + line * cTextHeight + ((rowLines[row] - cellData.size()) * cTextHeight / 2),
                          cellData[line].myText,
                          cellData[line].myRightAligned);
            }
        }
    }
}

size_t DisplayTable::nrRows() const
{
    return myTableData.size();
}

size_t DisplayTable::nrCols() const
{
    size_t cols = 0;

    for (TTableData::const_iterator iter = myTableData.begin(); iter != myTableData.end(); ++iter)
    {
        if (iter->size() > cols)
        {
            cols = iter->size();
        }
    }
    
    return cols;
}

size_t DisplayTable::getColWidth(cairo_t* surface, unsigned int col) const
{
    size_t colWidth = 0;

    for (TTableData::const_iterator iterRow = myTableData.begin(); iterRow != myTableData.end(); ++iterRow)
    {
        if (iterRow->size() > col)
        {
            for (TCellData::const_iterator iterLine = iterRow->at(col).begin(); iterLine != iterRow->at(col).end(); ++iterLine)
            {
                size_t width = getTextWidth(surface, iterLine->myText) + 2 * cHorizontalTextOffset;

                if (width > colWidth)
                {
                    colWidth = width;
                }
            }
        }
    }
    
    return colWidth;
}

size_t DisplayTable::getRowLines(unsigned int row) const
{
    size_t lines = 0;

    for (TRowData::const_iterator iter = myTableData[row].begin(); iter != myTableData[row].end(); ++iter)
    {
        if (iter->size() > lines)
        {
            lines = iter->size();
        }
    }

    return lines;
}

size_t DisplayTable::getTextWidth(cairo_t* surface, const string& text)
{
    cairo_text_extents_t extents;
    cairo_text_extents(surface, text.c_str(), &extents);

    return extents.width;
}

void DisplayTable::line(cairo_t* surface, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
    cairo_move_to(surface, x1, y1);
    cairo_line_to(surface, x2, y2);
    cairo_set_line_width(surface, 1);
    cairo_stroke(surface);
}

void DisplayTable::writeText(cairo_t* surface, unsigned int x, unsigned int y, const string& text, bool rightAligned)
{
    cairo_text_extents_t extents;
    cairo_text_extents(surface, text.c_str(), &extents);

    if (rightAligned)
    {
        cairo_move_to(surface, x - extents.width, y + cTextHeight);
    }
    else
    {
        cairo_move_to(surface, x, y + cTextHeight);
    }

    cairo_show_text(surface, text.c_str());
}
