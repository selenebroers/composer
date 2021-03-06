/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Code has been modified from the original.
****************************************************************************/

#include <QDialog>
#include <QWidget>
#include <QListWidget>
#include <QTextCodec>
#include <QLabel>
#include <QVBoxLayout>
#include <QByteArray>
#include <QFile>

#include <iostream>

class TextCodecSelector : public QDialog {
	Q_OBJECT
public:
	TextCodecSelector(QWidget* parent = 0)
		: QDialog(parent)
	{
		setWindowTitle(tr("Unknown encoding"));
		list = new QListWidget(this);
		connect(list, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(accept()));
		connect(list, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(accept()));
		codecs = QTextCodec::availableCodecs();
		qSort(codecs);
		list->addItem(tr("Automatic"));
		foreach (QByteArray n, codecs) {
			list->addItem(n);
		}
		QLabel *label = new QLabel(tr("<qt>Choose the encoding for this file:</qt>"));
		label->setWordWrap(true);
		QVBoxLayout *vb = new QVBoxLayout(this);
		vb->addWidget(label);
		vb->addWidget(list);
		setLayout(vb);
	}

	QTextCodec *selection(QByteArray ba) const
	{
		int n = list->currentRow();
		if (n < 0) {
			return 0;
		} else if (n == 0) {
			// Automatic... just try them all and pick the one that can convert
			// in and out without losing anything with the smallest intermediate length.
			QTextCodec *best = 0;
			int shortest = INT_MAX;
			foreach (QByteArray name, codecs) {
				QTextCodec* c = QTextCodec::codecForName(name);
				QString enc = c->toUnicode(ba);
				if (c->fromUnicode(enc) == ba) {
					std::cout << QString(c->name()).toStdString() << std::endl;
					if (enc.length() < shortest) {
						best = c;
						shortest = enc.length();
					}
				}
			}
			return best;
		} else {
			return QTextCodec::codecForName(codecs.at(n-1));
		}
	}

	static QTextCodec* codecForContent(QByteArray ba, QWidget *parent = 0)
	{
		TextCodecSelector tcs(parent);
		tcs.setModal(true);
		tcs.exec();
		if (tcs.result())
			return tcs.selection(ba);
		return 0;
	}

	static QString readAllAndHandleEncoding(QFile &file, QWidget *parent = 0)
	{
		QString data = "";
		QByteArray ba = file.readAll();
		if (!ba.isEmpty()) {
			data = QString::fromUtf8(ba, ba.size());
			if (data.toUtf8().size() != ba.size()) {
				// Not UTF-8 :(
				data = QString::fromLatin1(ba, ba.size());
				if (data.toLatin1().size() != ba.size()) {
					// Not Latin1 :(
					data = QString::fromLocal8Bit(ba, ba.size());
					if (data.toLocal8Bit().size() != ba.size()) {
						// Not Local 8-bit :(
						QTextCodec* codec = codecForContent(ba, parent);
						if (codec) data = codec->toUnicode(ba);
					}
				}
			}
		}
		return data;
	}

private:
	QListWidget *list;
	QList<QByteArray> codecs;
};
