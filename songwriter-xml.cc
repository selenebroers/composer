#include "songwriter.hh"
#include <QtXml/QDomDocument>
#include <QTextStream>
#include <iostream>

namespace {
	int sec2dur(double sec) {
		return sec; // FIXME: Implement
	}
}

void SingStarXMLWriter::writeXML() {
	QDomDocument doc("");
	QDomElement root = doc.createElement("MELODY");
	root.setAttribute("xmlns", "http://www.singstargame.com");
	root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root.setAttribute("Version", "1");
	root.setAttribute("Tempo", "120"); // FIXME!
	root.setAttribute("FixedTempo", "Yes");
	root.setAttribute("Resolution", "Demisemiquaver"); //?
	root.setAttribute("Genre", QString::fromStdString(s.genre));
	root.setAttribute("Year", QString::fromStdString(s.year));
	root.setAttribute("xsi:schemaLocation", "http://www.singstargame.com http://15GMS-SINGSQL/xml_schema/melody.xsd");
	root.setAttribute("m2xVersion", "060110"); //?
	root.setAttribute("audioVersion", "2"); //?
	doc.appendChild(root);

	QDomComment artistComment = doc.createComment(QString("Artist: ") + QString::fromStdString(s.artist));
	QDomComment titleComment = doc.createComment(QString("Title: ") + QString::fromStdString(s.title));
	root.appendChild(artistComment);
	root.appendChild(titleComment);

	int tracknum = 1;
	QDomElement trackElem = doc.createElement("TRACK");
	trackElem.setAttribute("Name", "Player1");
	trackElem.setAttribute("Artist", QString::fromStdString(s.artist));

	int sentencenum = 1;
	QDomElement sentenceElem = doc.createElement("SENTENCE"); // FIXME: Should there be Singer and Part attributes?
	QDomComment sentenceComment = doc.createComment(QString("Track %1, Sentence %2").arg(tracknum).arg(sentencenum));
	trackElem.appendChild(sentenceComment);

	// Iterate all notes
	Notes const& notes = s.getVocalTrack().notes;
	for (unsigned int i = 0; i < notes.size(); ++i) {
		Note const& n = notes[i];

		// SLEEP notes indicate sentence end
		if (n.type == Note::SLEEP) {
			trackElem.appendChild(sentenceElem);
			++sentencenum;
			sentenceElem = doc.createElement("SENTENCE");
			sentenceComment = doc.createComment(QString("Track %1, Sentence %2").arg(tracknum).arg(sentencenum));
			sentenceElem.appendChild(sentenceComment);
		}

		// Construct the note element
		QDomElement noteElem = doc.createElement("NOTE");
		noteElem.setAttribute("MidiNote", QString::number(n.note));
		noteElem.setAttribute("Duration", QString::number(sec2dur(n.length())));
		noteElem.setAttribute("Lyric", QString::fromStdString(n.syllable));
		if (n.type == Note::GOLDEN) noteElem.setAttribute("Bonus", "Yes");
		if (n.type == Note::FREESTYLE) noteElem.setAttribute("FreeStyle", "Yes");
		sentenceElem.appendChild(noteElem);

		// Construct a second note element, indicationg the pause before next note
		QDomElement pauseElem = doc.createElement("NOTE");
		double pauseLen = 1; // FIXME: This should be the duration from last note end to song end
		if (i < notes.size() - 1)
			pauseLen = notes[i+1].begin - n.end; // Difference to next note
		pauseElem.setAttribute("MidiNote", "0");
		pauseElem.setAttribute("Duration", QString::number(sec2dur(pauseLen)));
		pauseElem.setAttribute("Lyric", "");
		sentenceElem.appendChild(pauseElem);

	}

	root.appendChild(trackElem);

	// Get the xml data
	QString xml = doc.toString(4);
	// Write to file
	QFile f(path + "/notes.xml");
	if (f.open(QFile::WriteOnly)) {
		QTextStream out(&f);
		out << xml;
	} else throw std::runtime_error("Couldn't open target file notes.xml");
}
