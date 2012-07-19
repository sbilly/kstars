/***************************************************************************
                          KSParser.h  -  K Desktop Planetarium
                             -------------------
    begin                : 2012/24/06
    copyright            : (C) 2012 by Rishab Arora
    email                : ra.rishab@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSTARS_KSPARSER_H
#define KSTARS_KSPARSER_H
#include "ksfilereader.h"
#include <kstandarddirs.h>
#include <QList>
#include <QFile>
#include <QHash>
#include <QDebug>
#include <QVariant>

/**
 * @brief Generic class for text file parsers used in KStars.
 * Read rows using ReadCSVRow() regardless of the type of parser.
 **/
class KSParser {
 public:
    /**
     * @brief DataTypes for building sequence
     * D_QSTRING QString Type
     * D_INT Integer  Type
     * D_FLOAT Floating Point Type
     * D_DOUBLE Double PRecision Type
     * D_SKIP Not Needed. This string is not converted from QString
     * 
     **/
    enum DataTypes {
        D_QSTRING,
        D_INT,
        D_FLOAT,
        D_DOUBLE,
        D_SKIP
    };

    /**
     * @brief Returns a CSV parsing instance of a KSParser type object.
     *
     * @param filename Filename of source file
     * @param comment_char Character signifying a comment line
     * @param sequence QList of QPairs of the form "field name,data type" 
     * @param delimiter separate on which character. default ','
     **/
    KSParser(const QString &filename, const char comment_char,
             const QList< QPair<QString, DataTypes> > &sequence,
             const char delimiter = ',');

    /**
     * @brief Returns a Fixed Width parsing instance of a KSParser type object. 
     *
     * @param filename Filename of source file
     * @param comment_char Character signifying a comment line
     * @param sequence QList of QPairs of the form "field name,data type" 
     * @param widths width sequence. Last value is line.length() by default
     *               Hence, sequence.length() should be (width.length()+1)
     **/
    KSParser(const QString &filename, const char comment_char,
             const QList< QPair<QString, DataTypes> > &sequence,
             const QList<int> &widths);

    /**
     * @brief Generic function used to read the next row of a text file.
     * The contructor changes the function pointer to the appropriate function.
     * Returns the row as <"column name", value>
     *
     * @return QHash< QString, QVariant >
     **/
    QHash<QString, QVariant>  ReadNextRow();

    /**
     * @brief Returns True if there are more rows to be read
     *
     * @return bool
     **/
    const bool HasNextRow();

    /**
     * @brief Wrapper function for KSFileReader setProgress
     *
     * @param msg What message to display
     * @param total_lines Total number of lines in file
     * @param step_size Size of step in emitting progress  
     * @return void
     **/
    void SetProgress(QString msg, int total_lines, int step_size);

    /**
     * @brief Wrapper function for KSFileReader showProgress
     *
     * @return void
     **/
    void ShowProgress();

 private:
    /**
     * @brief Function Pointer used by ReadNextRow 
     * to call the appropriate function among ReadCSVRow and ReadFixedWidthRow
     *
     * @return QHash< QString, QVariant >
     **/
    QHash<QString, QVariant> (KSParser::*readFunctionPtr)();

    /**
     * @brief Returns a single row from CSV. 
     * If HasNextRow is false, returns a row with default values.
     *
     * @return QHash< QString, QVariant >
     **/
    QHash<QString, QVariant> ReadCSVRow();

    /**
     * @brief Returns a single row from Fixed Width File.
     * If HasNextRow is false, returns a row with default values.
     *
     * @return QHash< QString, QVariant >
     **/
    QHash<QString, QVariant> ReadFixedWidthRow();

    /**
     * @brief Returns a default value row.
     * Values are according to the current assigned sequence.
     *
     * @return QHash< QString, QVariant >
     **/
    QHash<QString, QVariant> DummyRow();

    KSFileReader file_reader_;
    QString filename_;
    int current_row_id_;
    bool more_rows_;
    char comment_char_;

    QList< QPair<QString, DataTypes> > name_type_sequence_;
    QList<int> width_sequence_;
    char delimiter_;
};

#endif  // KSTARS_KSPARSER_H
