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

#include "imageresult.h"
using namespace Cantor;

#include <QImage>
#include <QImageWriter>
#include <KZip>
#include <QDebug>
#include <QBuffer>
#include <QTemporaryFile>

class Cantor::ImageResultPrivate
{
  public:
    ImageResultPrivate() = default;

    QUrl url;
    QImage img;
    QString alt;
};

ImageResult::ImageResult(const QUrl &url, const QString& alt) :  d(new ImageResultPrivate)
{
    d->url=url;
    d->alt=alt;
}

Cantor::ImageResult::ImageResult(const QImage& image, const QString& alt) :  d(new ImageResultPrivate)
{
    d->img=image;
    d->alt=alt;

    QTemporaryFile imageFile;
    imageFile.setAutoRemove(false);
    if (imageFile.open())
    {
        d->img.save(imageFile.fileName(), "PNG");
        d->url = QUrl::fromLocalFile(imageFile.fileName());
    }
}

ImageResult::~ImageResult()
{
    delete d;
}

QString ImageResult::toHtml()
{
    return QStringLiteral("<img src=\"%1\" alt=\"%2\"/>").arg(d->url.toLocalFile(), d->alt);
}

QString ImageResult::toLatex()
{
    return QStringLiteral(" \\begin{center} \n \\includegraphics[width=12cm]{%1} \n \\end{center}").arg(d->url.fileName());
}

QVariant ImageResult::data()
{
    if(d->img.isNull())
        d->img.load(d->url.toLocalFile());

    return QVariant(d->img);
}

QUrl ImageResult::url()
{
    return d->url;
}

int ImageResult::type()
{
    return ImageResult::Type;
}

QString ImageResult::mimeType()
{
    const QList<QByteArray> formats=QImageWriter::supportedImageFormats();
    QString mimetype;
    foreach(const QByteArray& format, formats)
    {
        mimetype+=QLatin1String("image/"+format.toLower()+' ');
    }
    qDebug()<<"type: "<<mimetype;

    return mimetype;
}

QDomElement ImageResult::toXml(QDomDocument& doc)
{
    qDebug()<<"saving imageresult "<<toHtml();
    QDomElement e=doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("image"));
    e.setAttribute(QStringLiteral("filename"), d->url.fileName());
    if (!d->alt.isEmpty())
        e.appendChild(doc.createTextNode(d->alt));
    qDebug()<<"done";

    return e;
}

QJsonValue Cantor::ImageResult::toJupyterJson()
{
    QJsonObject root;

    if (executionIndex() != -1)
    {
        root.insert(QLatin1String("output_type"), QLatin1String("execute_result"));
        root.insert(QLatin1String("execution_count"), executionIndex());
    }
    else
        root.insert(QLatin1String("output_type"), QLatin1String("display_data"));

    QJsonObject data;
    data.insert(QLatin1String("text/plain"), toJupyterMultiline(d->alt));

    QImage image;
    if (d->img.isNull())
        image.load(d->url.toLocalFile());
    else
        image = d->img;

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    data.insert(QLatin1String("image/png"), QString::fromLatin1(ba.toBase64()));

    root.insert(QLatin1String("data"), data);

    QJsonObject metadata(jupyterMetadata());
    QJsonObject size;
    size.insert(QLatin1String("width"), image.size().width());
    size.insert(QLatin1String("height"), image.size().height());
    metadata.insert(QLatin1String("image/png"), size);
    root.insert(QLatin1String("metadata"), metadata);

    return root;
}

void ImageResult::saveAdditionalData(KZip* archive)
{
    archive->addLocalFile(d->url.toLocalFile(), d->url.fileName());
}

void ImageResult::save(const QString& filename)
{
    //load into memory and let Qt save it, instead of just copying d->url
    //to give possibility to convert file format
    QImage img=data().value<QImage>();

    img.save(filename);
}

