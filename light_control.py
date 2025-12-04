from tkinter import *
from tkinter import ttk # Для более красивых виджетов
import paho.mqtt.client as mqtt

# --- НАСТРОЙКИ MQTT ---
BROKER = "m3.wqtt.ru"
PORT = 14317
USER = "u_DT7B0K"
PASS = "r9fypHVG"

# Топики
TOPIC_LAMP_1 = "/topic/relay1"
TOPIC_LAMP_2 = "/topic/relay2"
TOPIC_LAMP_3 = "/topic/relay3"

TOPIC_BRIGHT_1 = "/topic/brightness1"
TOPIC_BRIGHT_2 = "/topic/brightness2"
TOPIC_BRIGHT_3 = "/topic/brightness3"

# --- ИНИЦИАЛИЗАЦИЯ MQTT ---
client = mqtt.Client()
client.username_pw_set(USER, PASS)

try:
    client.connect(BROKER, PORT, 60)
    client.loop_start() # Запускаем цикл обработки сети в фоне
    print("Подключено к MQTT брокеру")
except Exception as e:
    print(f"Ошибка подключения к MQTT: {e}")

# --- ЛОГИКА ПРИЛОЖЕНИЯ ---

# Состояния ламп (храним, включены они или нет)
lamp_states = [False, False, False] 

def toggle_lamp(index, btn_ref):
    """
    Функция переключения лампы (ВКЛ/ВЫКЛ).
    index: номер лампы (0, 1, 2)
    btn_ref: ссылка на кнопку, чтобы поменять её цвет и текст
    """
    current_state = lamp_states[index]
    new_state = not current_state
    lamp_states[index] = new_state
    
    # Определяем топик в зависимости от индекса
    topics = [TOPIC_LAMP_1, TOPIC_LAMP_2, TOPIC_LAMP_3]
    topic = topics[index]

    if new_state:
        # Включаем
        client.publish(topic, "ON")
        # Меняем вид кнопки 
        btn_ref.configure(text="ВКЛЮЧЕНО", bg="green", fg="white")
    else:
        # Выключаем
        client.publish(topic, "OFF")
        btn_ref.configure(text="ВЫКЛЮЧИТЬ", bg="red", fg="white")

def change_brightness(val, index):
    """
    Функция изменения яркости.
    val: значение от шкалы (строка)
    index: номер лампы
    """
    brightness_val = int(float(val))
    
    topics = [TOPIC_BRIGHT_1, TOPIC_BRIGHT_2, TOPIC_BRIGHT_3]
    topic = topics[index]
    
    # Отправляем значение яркости (0-100)
    client.publish(topic, str(brightness_val))
    print(f"Лампа {index+1} Яркость: {brightness_val}")

# --- СОЗДАНИЕ ГРАФИЧЕСКОГО ИНТЕРФЕЙСА (GUI) ---

window = Tk()
window.title("Панель управления светом")
window.geometry('500x300') # Настройка размеров окна

# Создаем заголовки таблицы
Label(window, text="Название", font=("Arial", 12, "bold")).grid(column=0, row=0, padx=10, pady=10)
Label(window, text="Переключатель", font=("Arial", 12, "bold")).grid(column=1, row=0, padx=10, pady=10)
Label(window, text="Яркость", font=("Arial", 12, "bold")).grid(column=2, row=0, padx=10, pady=10)

# --- СТРОКА 1: ЛАМПА 1 ---
lbl1 = Label(window, text="Лампа 1", font=("Arial", 10))
lbl1.grid(column=0, row=1, padx=10, pady=10)

# Кнопка (Button) с командой. Используем lambda, чтобы передать аргументы в функцию
btn1 = Button(window, text="ВЫКЛЮЧИТЬ", bg="red", fg="white", width=12)
btn1.configure(command=lambda: toggle_lamp(0, btn1)) 
btn1.grid(column=1, row=1)

# Ползунок (Scale)
scale1 = Scale(window, from_=0, to=100, orient=HORIZONTAL, length=150)
scale1.set(100) # Значение по умолчанию
scale1.configure(command=lambda val: change_brightness(val, 0))
scale1.grid(column=2, row=1)


# --- СТРОКА 2: ЛАМПА 2 ---
lbl2 = Label(window, text="Лампа 2", font=("Arial", 10))
lbl2.grid(column=0, row=2, padx=10, pady=10)

btn2 = Button(window, text="ВЫКЛЮЧИТЬ", bg="red", fg="white", width=12)
btn2.configure(command=lambda: toggle_lamp(1, btn2))
btn2.grid(column=1, row=2)

scale2 = Scale(window, from_=0, to=100, orient=HORIZONTAL, length=150)
scale2.set(100)
scale2.configure(command=lambda val: change_brightness(val, 1))
scale2.grid(column=2, row=2)


# --- СТРОКА 3: ЛАМПА 3 ---
lbl3 = Label(window, text="Лампа 3", font=("Arial", 10))
lbl3.grid(column=0, row=3, padx=10, pady=10)

btn3 = Button(window, text="ВЫКЛЮЧИТЬ", bg="red", fg="white", width=12)
btn3.configure(command=lambda: toggle_lamp(2, btn3))
btn3.grid(column=1, row=3)

scale3 = Scale(window, from_=0, to=100, orient=HORIZONTAL, length=150)
scale3.set(100)
scale3.configure(command=lambda val: change_brightness(val, 2))
scale3.grid(column=2, row=3)

# Запуск основного цикла (mainloop)
window.mainloop()