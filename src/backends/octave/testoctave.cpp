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

#include "testoctave.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "imageresult.h"
#include "textresult.h"
#include "epsresult.h"
#include "completionobject.h"

#include "octaveexpression.h"

#include <QDebug>

QString TestOctave::backendName()
{
    return QLatin1String("octave");
}

void TestOctave::testSimpleCommand()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( cleanOutput( e->result()->toHtml() ), QLatin1String("ans =  4") );
}
void TestOctave::testMultilineCommand()
{
    Cantor::Expression* e=evalExp( QLatin1String("a = 2+2, b = 3+3") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QString result=e->result()->toHtml();

    QCOMPARE( cleanOutput(result ), QLatin1String("a =  4\nb =  6") );
}

void TestOctave::testCommandQueue()
{
    Cantor::Expression* e1=session()->evaluateExpression(QLatin1String("0+1"));
    Cantor::Expression* e2=session()->evaluateExpression(QLatin1String("1+1"));
    Cantor::Expression* e3=evalExp(QLatin1String("1+2"));

    QVERIFY(e1!=nullptr);
    QVERIFY(e2!=nullptr);
    QVERIFY(e3!=nullptr);

    QVERIFY(e1->result());
    QVERIFY(e2->result());
    QVERIFY(e3->result());

    QCOMPARE(cleanOutput(e1->result()->toHtml()), QLatin1String("ans =  1"));
    QCOMPARE(cleanOutput(e2->result()->toHtml()), QLatin1String("ans =  2"));
    QCOMPARE(cleanOutput(e3->result()->toHtml()), QLatin1String("ans =  3"));
}

void TestOctave::testVariableDefinition()
{
    Cantor::Expression* e = evalExp(QLatin1String("testvar = 1"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(cleanOutput(e->result()->toHtml()), QLatin1String("testvar =  1"));
}

void TestOctave::testMatrixDefinition()
{
    Cantor::Expression* e = evalExp(QLatin1String(
        "M = [1, 2, 3;"\
        "     4, 5, 6;"\
        "     7, 8, 9;]"
    ));

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(e->result()->type(), (int) Cantor::TextResult::Type);

    Cantor::TextResult* result = static_cast<Cantor::TextResult*>(e->result());
    QCOMPARE(result->plain(), QLatin1String(
        "M =\n"\
        "\n"
        "   1   2   3\n"\
        "   4   5   6\n"\
        "   7   8   9"
    ));
}

void TestOctave::testSimpleExpressionWithComment()
{
    Cantor::Expression* e = evalExp(QLatin1String("s = 1234 #This is comment"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(cleanOutput(e->result()->toHtml()), QLatin1String("s =  1234"));
}

void TestOctave::testCommentExpression()
{
    // https://bugs.kde.org/show_bug.cgi?id=401893
    QSKIP("Skip, until we fix the bug #401893");
    Cantor::Expression* e = evalExp(QLatin1String("#Only comment"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QCOMPARE(e->results().size(), 0);
}

void TestOctave::testCompletion()
{
    QSKIP("Skip, until solve strange fail of this test with anomaly output (double prompt with missing command)");
    Cantor::CompletionObject* help = session()->completionFor(QLatin1String("as"), 2);
    waitForSignal(help, SIGNAL(fetchingDone()));

    // Checks some completions for this request (but not all)
    // This correct for Octave 4.2.2 at least (and another versions, I think)
    const QStringList& completions = help->completions();
    qDebug() << completions;
    QVERIFY(completions.contains(QLatin1String("asin")));
    QVERIFY(completions.contains(QLatin1String("asctime")));
    QVERIFY(completions.contains(QLatin1String("asec")));
    QVERIFY(completions.contains(QLatin1String("assert")));
}

void TestOctave::testPlot()
{
    Cantor::Expression* e=evalExp( QLatin1String("cantor_plot2d('sin(x)', 'x', -10,10);") );

    int cnt=0;
    //give some time to create the image, but at most 5sec
    while(e->result()==nullptr||e->result()->type()!=OctavePlotResult::Type )
    {
        QTest::qWait(250);
        cnt+=250;
        if(cnt>5000)
            break;
    }

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( e->result()->type(), (int) OctavePlotResult::Type );
    QVERIFY( !e->result()->data().isNull() );
}

void TestOctave::testInvalidSyntax()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2*+.") );

    QVERIFY( e!=nullptr );
    QCOMPARE( e->status(), Cantor::Expression::Error );
}

QTEST_MAIN( TestOctave )

