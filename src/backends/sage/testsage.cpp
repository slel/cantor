/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "testsage.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "imageresult.h"

#include <kdebug.h>

QString TestSage::backendName()
{
    return "sage";
}

void TestSage::testSimpleCommand()
{
    Cantor::Expression* e=evalExp( "2+2" );

    QVERIFY( e!=0 );
    QVERIFY( e->result()!=0 );
    QCOMPARE( e->result()->toHtml(), QString("4") );
}
void TestSage::testMultilineCommand()
{
    Cantor::Expression* e=evalExp( "2+2 \n simplify(1 - x + x)" );

    QVERIFY( e!=0 );
    QVERIFY( e->result()!=0 );
    QCOMPARE( e->result()->toHtml(), QString("4<br/>\n1") );
}

void TestSage::testDefineFunction()
{
    const char* cmd="def func1(param) : \n" \
                    "    return param*param\n\n";

    Cantor::Expression* e1=evalExp( cmd );

    QVERIFY( e1!=0 );
    QVERIFY( e1->result()!=0 );

    Cantor::Expression* e2=evalExp( "func1(2)" );
    QVERIFY( e2!=0 );
    QVERIFY( e2->result()!=0 );
    QCOMPARE( e2->result()->toHtml(), QString("4") );
}

void TestSage::testPlot()
{
    Cantor::Expression* e=evalExp( "plot(sin(x))" );

    QVERIFY( e!=0 );
    QVERIFY( e->result()!=0 );
    QVERIFY( e->result()->type()==Cantor::ImageResult::Type );
    QVERIFY( !e->result()->data().isNull() );
}

void TestSage::testInvalidSyntax()
{
    Cantor::Expression* e=evalExp( "2+2*(" );

    QVERIFY( e!=0 );
    QVERIFY( e->errorMessage()== "Syntax Error" );
}

void TestSage::testNoOutput()
{
    Cantor::Expression* e=evalExp(  "f(x)=x^2+3*x+2\nf(0)" );

    QVERIFY( e!=0 );
    QVERIFY( e->result() != 0 );
    QCOMPARE( e->result()->toHtml(), QString("2") );
}

QTEST_MAIN( TestSage )

#include "testsage.moc"