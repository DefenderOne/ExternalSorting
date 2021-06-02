// Есаков, 61гр 2 курс. Естественное двухпутевое сбалансированное однофазное равномерное слияние

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

using namespace std;

struct Sequence
{
	int elem = 0;
	fstream file;
	bool eof = false;
	bool eor = false;
	string filename;
};

void readNext(Sequence& s)
{
	int prev = s.elem;
	if (s.file >> s.elem)
	{
		if (prev > s.elem)
		{
			s.eor = true;
		}
	}
	else // Файл кончился
	{
		s.eof = true;
		s.eor = true;
	}
}

void startRead(Sequence& s)
{
	s.file.open(s.filename, fstream::in);
	s.file >> s.elem;
}

void startWrite(Sequence& s)
{
	s.file.open(s.filename, fstream::out);
	s.eor = false;
	s.eof = false;
}

void copyElem(Sequence& s1, Sequence& s2)
{
	s2.elem = s1.elem;
	s2.file << s1.elem << ' ';
	readNext(s1);
}

void copyRun(Sequence& s1, Sequence& s2)
{
	while (!s1.eor)
	{
		copyElem(s1, s2);
	}
}

bool distNextSeq(ifstream& source, fstream& target, int& start)
{
	int prev = start;
	int current;
	target << prev << ' '; // Первый элемент сразу выводим
	bool readed = bool(source >> current);
	if (readed && prev <= current) 
	{
		do
		{
			target << current << ' ';
			prev = current;
			readed = bool(source >> current);
		} while (readed && prev <= current);
	}
	start = current;
	return readed;
}

bool distribution(ifstream& source, Sequence*& target)
{
	// Связывание
	target = new Sequence[4];
	bool simple = true; // Если занесли всё только в один файл, то последовательность уже отсортирована
	for (int i = 0; i < 4; i++)
	{
		target[i].filename = "tmp" + to_string(i) + ".txt";
		target[i].file.open(target[i].filename, fstream::out);
		target[i].eof = false;
		target[i].eor = false;
	}
	target[2].file.close();
	target[3].file.close();

	int start;
	source >> start;
	bool extraRead = true; // Конец файла определяем по результату считывания >>, если после чтения (>>) false, то дошли до конца
	while (extraRead)
	{
		extraRead = distNextSeq(source, target[0].file, start);
		if (extraRead)
		{
			simple = false;
			extraRead = distNextSeq(source, target[1].file, start);
		}
	}

	target[0].file.close();
	target[1].file.close();
	return simple;
}

void mergePair(Sequence& s1, Sequence& s2, Sequence& to)
{
	s1.eor = s2.eor = false;
	if (!s1.eof || !s2.eof)
	{
		while (!s1.eor && !s2.eor && !s1.eof && !s2.eof)
		{
			// Помещаем меньший элемент
			if (s1.elem <= s2.elem)
				copyElem(s1, to);
			else
				copyElem(s2, to);
		}

		if (!s1.eof || !s2.eof)
		{
			if (s1.eor || s1.eof)
				copyRun(s2, to);
			else // s2.eor || s2.eof
				copyRun(s1, to);
		}
	}
}

void reset(Sequence* target)
{
	for (int i = 0; i < 4; i++)
	{
		target[i].file.close();
		target[i].eof = false;
		target[i].eor = false;
	}
}

void open(Sequence* target, bool up)
{
	if (up)
	{
		startRead(target[0]);
		startRead(target[1]);
		startWrite(target[2]);
		startWrite(target[3]);
	}
	else
	{
		startRead(target[2]);
		startRead(target[3]);
		startWrite(target[0]);
		startWrite(target[1]);
	}
}

void mergeProcess(Sequence* tmp, bool up, bool& end)
{
	int in1, in2;
	int out1, out2;

	if (up) // Выбираем направление
	{
		in1 = 0;
		in2 = 1;
		out1 = 2;
		out2 = 3;
	}
	else
	{
		in1 = 2;
		in2 = 3;
		out1 = 0;
		out2 = 1;
	}

	mergePair(tmp[in1], tmp[in2], tmp[out1]);
	if (tmp[in1].eof && tmp[in2].eof) end = true;

	while (tmp[in1].eof == false || tmp[in2].eof == false)
	{
		mergePair(tmp[in1], tmp[in2], tmp[out2]);
		mergePair(tmp[in1], tmp[in2], tmp[out1]);
	}
}

void sort(ifstream& source)
{
	Sequence* tmp = nullptr;
	bool up = true;
	bool end = distribution(source, tmp);
	while (!end)
	{
		open(tmp, up);

		mergeProcess(tmp, up, end);

		up = !up;

		reset(tmp);
	}

	remove("out.txt");
	if (up)
	{
		rename("tmp0.txt", "out.txt");
		remove("tmp1.txt");
		remove("tmp2.txt");
		remove("tmp3.txt");
	}
	else
	{
		rename("tmp2.txt", "out.txt");
		remove("tmp0.txt");
		remove("tmp1.txt");
		remove("tmp3.txt");
	}
	delete[] tmp;
}

int main()
{
	ifstream source("source.txt");

	if (source.is_open())
	{
		cout << "Result in out.txt\n";
		sort(source);
	}

	source.close();

	cin.get();
	return 0;
}