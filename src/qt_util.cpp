#include "qt_util.h"

#include <QEvent>




Utility::Event_filter::Event_filter(QObject *parent)
	: QObject(parent) {}

Utility::Event_filter::Event_filter(QObject *parent, std::function<bool(QEvent *)> function)
	: QObject(parent)
	, callbacks({std::move(function)}) {}

void Utility::Event_filter::add_callback(std::function<bool(QEvent *)> function) {
	callbacks.emplace_back(std::move(function));
}

void Utility::Event_filter::clear() {
	callbacks.clear();
}

bool Utility::Event_filter::eventFilter(QObject *object, QEvent *ev) {
    (void)object;
	bool retval = false;
	for (auto &function : callbacks) {
		retval |= function(ev);
	}
	return retval;
}
