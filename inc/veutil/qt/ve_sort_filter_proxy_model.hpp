#pragma once

#include <qsortfilterproxymodel.h>

class VeSortFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
	Q_PROPERTY(QAbstractItemModel *model READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)
	Q_PROPERTY(bool naturalSort READ naturalSort WRITE setNaturalSort NOTIFY naturalSortChanged)

public:
	Q_INVOKABLE void setSourceModel(QAbstractItemModel *sourceModel) override {
		QSortFilterProxyModel::setSourceModel(sourceModel);
		emit sourceModelChanged();
		sort(0);
	}

	void setNaturalSort(bool natural) {
		if (mNaturalSort == natural)
			return;
		mNaturalSort = natural;
		sort(0);
		emit naturalSortChanged();
	}

	bool naturalSort() { return mNaturalSort; }

signals:
	void naturalSortChanged();
	void sourceModelChanged();

protected:
	bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override
	{
		if (!mNaturalSort)
			return QSortFilterProxyModel::lessThan(source_left, source_right);

		QString l = (source_left.model() ? source_left.model()->data(source_left, sortRole()).toString() : "");
		QString r = (source_right.model() ? source_right.model()->data(source_right, sortRole()).toString() : "");

		if (sortCaseSensitivity() == Qt::CaseInsensitive) {
			l = l.toLower();
			r = r.toLower();
		}

		return naturalSortIsLessThan(l, r);
	}

private:
	bool naturalSortIsLessThan(QString left, QString right) const
	{
		int len = qMin(left.length(), right.length());
		for (int i = 0; i < len; i++) {

			// Make sure that "MPTT 2" is before "MPPT 10" etc, since alfabetically
			// "MPPT 2" will be after "MPPT 10".

			if (left[i].isDigit()) {
				if (!right[i].isDigit())
					return true;

				QString ln, rn;
				int n = i;
				while (left[n].isDigit() && n < left.length())
					ln += left[n++];

				n = i;
				while (right[n].isDigit() && n < right.length())
					rn += right[n++];

				unsigned l = ln.toUInt();
				unsigned r = rn.toUInt();

				// In case the numbers are equal, continue alphabetically.
				// mind it: the length of the string might not be equal, e.g.
				// 00001 might have been compared with 1.
				if (l == r) {
					left = left.mid(i + ln.length());
					right = right.mid(i + rn.length());
					i = 0;
					len = qMin(left.length(), right.length());
				} else {
					return l < r;
				}
			}

			if (left[i] < right[i])
					return true;
			if (left[i] > right[i])
					return false;
		}

		return left.length() < right.length();
	}

	bool mNaturalSort = false;
};
