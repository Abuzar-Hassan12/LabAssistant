#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <fstream>
#include <string>

using namespace std;

class Student {
public:
    string firstName;
    string lastName;
    int enterTime = 0;
    int numQuestions = 0;
    int unanswered = -1;
    int laptopSerialNum = -1;
    bool isVisitedTA = false;
    bool isDone = false;

    //copy constructor
    Student(const Student& s) {
        this->firstName = s.firstName;
        this->lastName = s.lastName;
        this->enterTime = s.enterTime;
        this->numQuestions = s.numQuestions;
        this->unanswered = s.unanswered;
        this->laptopSerialNum = s.laptopSerialNum;
        this->isVisitedTA = s.isVisitedTA;
        this->isDone = s.isDone;
    }

    //default constructor
    Student() {
        firstName = "";
        lastName = "";
        enterTime = 0;
        numQuestions = 0;
        unanswered = -1;
        laptopSerialNum = -1;
        isVisitedTA = false;
        isDone = false;
    }
};

class TA {
public:
    string name; // only use first names for the TAs 
    //int startTimes = 0;
    //int endTimes = 0;
    int startTimes[3];
    int endTimes[3];
    Student studentWithTA;
    int minute;
    bool isActive;
};

class Laptop {
public:
    int serialNum;
    bool isAvailable;

    Laptop(int serial) : serialNum(serial), isAvailable(true) {}
};

class LabSimulation {
private:
    vector<Student*> studentsPerDay;
    queue<Student> laptopLine;
    queue<Student> TALine;
    stack<int> Laptops;
    vector<TA> tas;
    int currentTime;
    fstream inputFile;
    Student* students;
    int noStudents;
    int noLaptops;
    int noDepartments;
    TA* ta;
    int noTA;
    int i;
    int j;
    TA currentTA;
    int closingTime;
    int happy;
    int studentSize[3];
public:
    LabSimulation();

    void simulateDay(int);

    void printSimulationSummary(int);

private:
    void handleEntering();
    int getEndTime(int);
    void handleLaptopQueue();
    void handleTAQueue();
};

string formatTime(int minutesAfterNoon);

void readingAllData(fstream& inputFile, stack<int>& Laptops, Student*& students, TA*& ta, int& noStudents, int& noLaptops, int& noDepartments, int& noTA, vector<Student*>& studentsOnDays, int*);

LabSimulation::LabSimulation() {
    currentTime = 0;
    i = j = 0;
    inputFile.open("input.txt", ios::in);
    students = new Student[noStudents];
    ta = new TA[noTA];
    closingTime = 0;
    happy = 0;
    readingAllData(inputFile, Laptops, students, ta, noStudents, noLaptops, noDepartments, noTA, studentsPerDay, studentSize);
    cout << "no TAs are " << noTA << endl;
}

void LabSimulation::simulateDay(int day) {
    day -= 1; //as array starts from 0 index and taking days from day 1 onwards

    currentTime = 0;
    students = studentsPerDay[day];
    noStudents = studentSize[day];
    closingTime = getEndTime(day);
    while (currentTime < closingTime) {
        for (int l = 0; l < noTA; l++) {
            if (ta[l].startTimes[day] == currentTime && ta[l].startTimes[day] != ta[l].endTimes[day]) {
                ofstream outputFile("output.txt", ios_base::app);
                outputFile << formatTime(currentTime) << " :  " << ta[l].name << " has begun lab hours" << endl;
                outputFile.close();
                currentTA = ta[l];
                ta[l].isActive = true;
            }
            else if (ta[l].endTimes[day] == currentTime)
            {
                ofstream outputFile("output.txt", ios_base::app);
                outputFile << formatTime(currentTime) << " :  " << ta[j].name << " has ended lab hours" << endl;
                outputFile.close();
                //if (currentTime == ta[j].endTimes[day] && j == noTA - 1) //PS added ++ to currentTime
            }
        }

        handleEntering();
        handleLaptopQueue();
        handleTAQueue();
        //cout << currentTime << " till " << ta[noTA - 1].endTimes[day] << endl;
        currentTime++;
    }
    //As day over now emptying all remaining queues
    cout << formatTime(currentTime) << " :  " << "There are no TAs on duty.TA#007 is now closed." << endl;
    while (!TALine.empty())
    {
        if (closingTime - TALine.front().enterTime < 5) {
            TALine.front().isDone = true;
            TALine.push(TALine.front());
            TALine.pop();
        }
        laptopLine.push(TALine.front());
        TALine.pop();
    }

    while (!laptopLine.empty())
        handleLaptopQueue();
    printSimulationSummary(day);
}

void LabSimulation::printSimulationSummary(int day) {
    switch (day) {
    case 0:
        cout << "Monday's ";
        break;
    case 1:
        cout << "Tuesday's ";
        break;
    case 2:
        cout << "Wednesday's ";
        break;
    }
    ofstream outputFile("output.txt", ios_base::app);
    outputFile << "Lab Summary:\n" << "The TA Lab was open for " << currentTime / 60 << " hours and " << currentTime % 60 << " minutes.\n"
        << noStudents << " students visited the lab.Out of those students, only " << happy << " left happy." << " The remaining left frustrated.\n"
        << "Lesson Learned : Do not procrastinate!Start programs early!" << endl;
    outputFile.close();
}

int LabSimulation::getEndTime(int day) {
    int max = 0;
    for (int k = 0; k < noTA; k++)
        if (ta[k].endTimes[day] > max)
            max = ta[k].endTimes[day];
    return max;
}

void LabSimulation::handleEntering() {
    if (i < noStudents && students[i].enterTime == currentTime && students[i].laptopSerialNum == -1) {
        ofstream outputFile("output.txt", ios_base::app);
        outputFile << formatTime(currentTime) << " :  " << students[i].firstName << " has arrived in the TA#007" << endl;
        outputFile.close();
        laptopLine.push(students[i]);
        i++;
    }
}

void LabSimulation::handleLaptopQueue() {
    ofstream outputFile("output.txt", ios_base::app);
    if (!laptopLine.empty() && currentTime >= closingTime)//so student returning laptop
    {
        if (laptopLine.front().laptopSerialNum == -1)
            outputFile << formatTime(currentTime) << " :  " << laptopLine.front().firstName
            << " never even got a laptop and went home FRUSTRATED" << endl;
        else {
            outputFile << formatTime(currentTime) << " :  " << laptopLine.front().firstName
                << " has returned the laptop " << laptopLine.front().laptopSerialNum;
            if ((laptopLine.front().unanswered * 1.0) / laptopLine.front().numQuestions < 0.25) {
                outputFile << " and went home HAPPY" << endl;
                happy++;
            }
            else
                outputFile << " and went home FRUSTRATED" << endl;
        }
        Laptops.push(laptopLine.front().laptopSerialNum);
        laptopLine.pop();
        currentTime += 1;

    }

    else if (!laptopLine.empty() && laptopLine.front().isDone && currentTime - laptopLine.front().enterTime == 2)//so student returning laptop
    {
        outputFile << formatTime(currentTime) << " :  " << laptopLine.front().firstName
            << " has returned the laptop " << laptopLine.front().laptopSerialNum
            << " and went home HAPPY" << endl;
        Laptops.push(laptopLine.front().laptopSerialNum);
        laptopLine.pop();
        happy++;
        return;
    }

    else if (!laptopLine.empty() && currentTime - laptopLine.front().enterTime == 2) { // laptopLine.front().firstName == students[i].firstName && students[i].isVisitedTA == false &&
        //PS added check to only print and pop when time is +2 from arrival
        //currentTime += 2;
        laptopLine.front().enterTime += 2;
        outputFile << formatTime(currentTime) << " :  " << laptopLine.front().firstName
            << " has borrowed the laptop " << Laptops.top() << " and moved to the TA line" << endl;
        laptopLine.front().laptopSerialNum = Laptops.top();
        Laptops.pop();
        // laptopLine.front().unanswered = laptopLine.front().numQuestions;  BEFORE CODE
        laptopLine.front().unanswered = 1;

        TALine.push(laptopLine.front());
        //cout << TALine.front().enterTime << endl;
        //cout << "Queue: " << TALine.size() << endl;
        laptopLine.pop();
        outputFile.close();
    }
}
void LabSimulation::handleTAQueue()
{
    ofstream outputFile("output.txt", ios_base::app);
    //cout << "Queue: " << TALine.size() << endl;
    if (TALine.empty() || !currentTA.isActive)
        return;

    //cout << "Front waiting time: " << currentTime - TALine.front().enterTime << "qs: " << TALine.front().unanswered << endl;
    if (TALine.front().enterTime == currentTime)
    {
        outputFile << formatTime(currentTime) << " :  " << TALine.front().firstName
            << " is getting help from the " << currentTA.name << endl;
        TALine.front().unanswered--;
    }

    else if (TALine.front().unanswered == 0 && currentTime - TALine.front().enterTime == 5)
    {
        outputFile << formatTime(currentTime) << " :  " << TALine.front().firstName
            << " had no more question to answer and move to the laptop line" << endl;
        TALine.front().isDone = true;
        TALine.front().enterTime = currentTime;
        laptopLine.push(TALine.front());
        TALine.pop();
        handleLaptopQueue();
        if (TALine.empty())
            return;
        TALine.front().enterTime = currentTime;
        handleTAQueue();
    }
    //else if (TALine.front().unanswered > 0 && currentTime - TALine.front().enterTime == 5)//students[i].firstName == TALine.front().firstName && 
    //{
    //    outputFile << formatTime(currentTime) << " :  " << TALine.front().firstName
    //        << " had one question answered and gotten back to the line" << endl;
    //    //TALine.front().unanswered--;
    //    TALine.push(TALine.front());
    //    TALine.pop();//getting to  next person in queue
    //    if (TALine.empty())
    //        return;
    //    TALine.front().enterTime = currentTime;
    //    handleTAQueue();

    //}

    // REMOVE THIS PART ABOVE WHICH I HAVE COMMENTED
    outputFile.close();
}
string formatTime(int minutesAfterNoon) {
    int hours = minutesAfterNoon / 60;
    int minutes = minutesAfterNoon % 60;
    string period = (hours >= 12) ? " PM" : " AM";
    hours = (hours == 0 || hours == 12) ? 12 : hours % 12;
    return to_string(hours) + ":" + (minutes < 10 ? "0" : "") + to_string(minutes) + period;
}

void readingAllData(fstream& inputFile, stack<int>& Laptops, Student*& students, TA*& ta, int& noStudents, int& noLaptops, int& noDepartments, int& noTA, vector<Student*>& studentsPerDay, int* studentSize) {
    inputFile >> noLaptops;
    for (int i = 0; i < noLaptops; i++) {
        int serialNo;
        inputFile >> serialNo;
        Laptops.push(serialNo);
    }

    inputFile >> noTA;
    ta = new TA[noTA];
    for (int i = 0; i < noTA; i++) {
        inputFile >> ta[i].name >> ta[i].startTimes[0] >> ta[i].endTimes[0] >> ta[i].startTimes[1] >> ta[i].endTimes[1] >> ta[i].startTimes[2] >> ta[i].endTimes[2];
        //cout << ta[i].name;
    }

    inputFile >> noDepartments;
    inputFile >> noStudents;
    //cout << noDepartments << " and students = " << noStudents << endl;
    students = new Student[noStudents];
    studentSize[0] = noStudents;
    for (int i = 0; i < noStudents; i++) {
        inputFile >> students[i].enterTime >> students[i].firstName >> students[i].lastName >> students[i].numQuestions;
    }
    studentsPerDay.push_back(students);

    inputFile >> noStudents;
    //cout << noDepartments << " and students = " << noStudents << endl;
    students = new Student[noStudents];
    studentSize[1] = noStudents;
    for (int i = 0; i < noStudents; i++) {
        inputFile >> students[i].enterTime >> students[i].firstName >> students[i].lastName >> students[i].numQuestions;
    }
    studentsPerDay.push_back(students);

    inputFile >> noStudents;
    studentSize[2] = noStudents;
    //cout << noDepartments << " and students = " << noStudents << endl;
    students = new Student[noStudents];
    for (int i = 0; i < noStudents; i++) {
        inputFile >> students[i].enterTime >> students[i].firstName >> students[i].lastName >> students[i].numQuestions;
    }
    studentsPerDay.push_back(students);
    cout << studentsPerDay.size() << endl;
}



int main() {

    LabSimulation labSimulation;
    for (int i = 0; i < 3; i++)
    {
        if (i == 0)
        {
            ofstream outputFile("output.txt", ios_base::app);
            cout << "monday" << endl;
            outputFile << "Monday" << endl;

        }
        else if (i == 1)
        {
            cout << "press any for other day" << endl;
            cin.get();
            ofstream outputFile("output.txt", ios_base::app);
            cout << "Tuesday" << endl;
            outputFile << "Tuesday" << endl;
        }
        else if (i == 2)
        {
            cout << "press any for other day" << endl;
            cin.get();
            ofstream outputFile("output.txt", ios_base::app);

            cout << "Wednesday" << endl;
            outputFile << "Wednesday" << endl;
            outputFile.close();
        }
        labSimulation.simulateDay(i + 1);
        cout << endl;
    }
    //labSimulation.printSimulationSummary();

    return 0;
}