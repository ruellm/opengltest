#pragma once

template <class TArrayElement> 
class RecyclingArray
{
public:
	RecyclingArray() : _elements(0) { }
	RecyclingArray(int count) {
		Generate(count);
	}

	~RecyclingArray() {
		delete[] _elements;
		delete[] _active;
	}

	void Generate( int count ) {
		_count = count;
		_used_element_cnt = 0;
		_elements = new TArrayElement[count];
		_active = new bool[count];
		memset(_active, 0, count);
	}

	TArrayElement* GetNewElement() {
		if (GetNumFreeElements() < 1)
			return NULL;

		int found = -1;
		for (int i = 0; i < _count; i++) {
			if (!_active[i]) {
				found = i;
				break;
			}
		}

		if (found == -1) 
			return NULL;

		_used_element_cnt++;
		_active[found] = true;
		return &_elements[found];
	}

	void Delete(TArrayElement *elem)
	{
		// compute position based on pointer address
		if (_used_element_cnt == 0) return;
		
		int iElem = reinterpret_cast<int>(elem);
		iElem -= reinterpret_cast<int>(&_elements[0]);
		int index = iElem / sizeof(TArrayElement);
		if (index < 0 || index >= _count || !_active[index]) return;

		_active[index] = false;
		_used_element_cnt--;
	}

	int GetNumFreeElements() {
		return _count - _used_element_cnt;
	}

private:
	int				_count;
	int				_used_element_cnt;
	TArrayElement*	_elements;
	bool*			_active;
};