#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <queue>
#include <cstring>
#include <stdexcept>

using namespace std;


// Структура описывающая пациента: type: 0 - идет к стоматологу, 1 - к хирургу, 2 - к терапевту
struct patient_t {
    int type;
    int index;
};

// Объявляем все необходимые очереди
queue<patient_t> patient_queue;
queue<patient_t> dentist_queue;
queue<patient_t> surgeon_queue;
queue<patient_t> therapist_queue;
bool run = true;

// Структура для передачи параметров в функцию поток
struct args_t {
    pthread_mutex_t mutex;
};

// Функция потока для управления работой дежурных врачей
void* doctor_thread(void *args) {
    args_t *arg = (args_t*) args;
    pthread_mutex_t mutex = arg->mutex;
    while (true) {
        // Блокируем доуступ для операций с очередью
        pthread_mutex_lock(&mutex);
        if (patient_queue.size() > 0) {
            patient_t patient = patient_queue.front();
            if (patient.type == 0) {
                dentist_queue.push(patient);
                patient_queue.pop();
                cout << "В очереди к стоматологу " << dentist_queue.size() << " пациентов\n";
            } else if (patient.type == 1) {
                surgeon_queue.push(patient);
                patient_queue.pop();
                cout << "В очереди к хирургу " << surgeon_queue.size() << " пациентов\n";
            } else {
                therapist_queue.push(patient);
                patient_queue.pop();
                cout << "В очереди к терапевту " << therapist_queue.size() << " пациентов\n";
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 2 + 2);
    }
}

// Функция потока для управления работой стоматолога
void* dentist_thread(void *args) {
    args_t *arg = (args_t*) args;
    pthread_mutex_t mutex = arg->mutex;
    while (run || dentist_queue.size() > 0) {
        // Блокируем доуступ для операций с очередью
        pthread_mutex_lock(&mutex);
        if (dentist_queue.size() > 0) {
            patient_t patient = dentist_queue.front();
            dentist_queue.pop();
            cout << "Стоматолог принимает пациента номер " << patient.index << "\n";
            pthread_mutex_unlock(&mutex);
            sleep(rand() % 3 + 5);
            cout << "Стоматолог закончил принимать пациента номер " << patient.index << "\n";
        } else {
            pthread_mutex_unlock(&mutex);
        }
        sleep(1);
    }
}


// Функция потока для управления работой хирурга
void* surgeon_thread(void *args) {
    args_t *arg = (args_t*) args;
    pthread_mutex_t mutex = arg->mutex;
    while (run || surgeon_queue.size() > 0) {
        // Блокируем доуступ для операций с очередью
        pthread_mutex_lock(&mutex);
        if (surgeon_queue.size() > 0) {
            patient_t patient = surgeon_queue.front();
            surgeon_queue.pop();
            cout << "Хирург принимает пациента номер " << patient.index << "\n";
            pthread_mutex_unlock(&mutex);
            sleep(rand() % 3 + 5);
            cout << "Хирург закончил принимать пациента номер " << patient.index << "\n";
        } else {
            pthread_mutex_unlock(&mutex);
        }
        sleep(1);
    }
}

// Функция потока для управления работой терапевта
void* therapist_thread(void *args) {
    args_t *arg = (args_t*) args;
    pthread_mutex_t mutex = arg->mutex;
    while (run || therapist_queue.size() > 0) {
        // Блокируем доуступ для операций с очередью
        pthread_mutex_lock(&mutex);
        if (therapist_queue.size() > 0) {
            patient_t patient = therapist_queue.front();
            therapist_queue.pop();
            cout << "Терапевт принимает пациента номер " << patient.index << "\n";
            pthread_mutex_unlock(&mutex);
            sleep(rand() % 3 + 5);
            cout << "Терапевт закончил принимать пациента номер " << patient.index << "\n";
        } else {
            pthread_mutex_unlock(&mutex);
        }
        sleep(1);
    }
}


int main(int argc, char const *argv[]) {
    // Проверка на правильность ввода
    if (argc < 3) {
        cout << "Неправильное количество аргументов!\n";
        return 0;
    }
    int n;
    if (strcmp(argv[1], "-c") == 0) {
        try {
            n = atoi(argv[2]);
        } catch (exception e) {
            cout << "Неверное количество пациентов!\n";
            return 0;
        }
        if (n < 1) {
            cout << "Неверное количество пациентов!\n";
            return 0;
        }
    } else {
        cout << "Неправильное значение флага\n";
        return 0;
    }


    srand(time(nullptr));

    // Объявляем все необходимые потки
    pthread_t doctors[2];
    pthread_t specialists[3];
    // Создаем мьютекс для управления обращения к общим данным
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, nullptr);

    // Создаем объект аргуметов чтобы передать в потоки
    args_t args {
        mutex
    };

    // Объявляем все очереди
    patient_queue = queue<patient_t>();
    dentist_queue = queue<patient_t>();
    surgeon_queue = queue<patient_t>();
    therapist_queue = queue<patient_t>();

    // Создаем потоки для дежурных врачей
    pthread_create(&doctors[0], nullptr, doctor_thread, &args);
    pthread_create(&doctors[1], nullptr, doctor_thread, &args);

    // Создаем потоки для специалистов
    pthread_create(&specialists[0], nullptr, dentist_thread, &args);
    pthread_create(&specialists[1], nullptr, surgeon_thread, &args);
    pthread_create(&specialists[2], nullptr, therapist_thread, &args);
    
    // Главный поток будет создавать новых пациентов
    for (int i = 0; i < n; i++) {
        patient_t new_patient {
            rand() % 3, i + 1
        };
        cout << "Новый пациент: " << "<номер: " << i + 1 << ", специалист: ";
        switch(new_patient.type) {
            case 0:
                cout << "Стоматолог>\n";
                break;
            case 1:
                cout << "Хирург>\n";
                break;
            case 2:
                cout << "Терапевт>\n";
                break;
        }
        pthread_mutex_lock(&mutex);
        patient_queue.push(new_patient);
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 5 + 2);
    }
    
    pthread_mutex_lock(&mutex);
    run = false;
    pthread_mutex_unlock(&mutex);
    // После того как создали всех специалистов, ждем когда они закончат лечить своих пациентов
    pthread_join(specialists[0], nullptr);
    pthread_join(specialists[1], nullptr);
    pthread_join(specialists[2], nullptr);
    pthread_mutex_destroy(&mutex);
    return 0;
}