Qt-YandexTranslate
This project is dedicated to working with the Yandex.Translate and Yandex.Dictionary APIs in Qt using C++. The project was created solely to understand the structure of API requests to the specified services and is currently completed. With this code, you can:

1. Get a list of languages
2. Detect the language
3. Translate text
4. Get a dictionary entry for a word or phrase.


First of all, after downloading the repository, you should obtain API keys for 
Yandex.Translate and Yandex.Dictionary and specify them in the corresponding files.

The files `dictionary_api.h` and `dictionary_api.cpp` represent how Yandex.Dictionary API support was implemented. The `translate_api.h` and `translate_api.cpp` files represent how Yandex.Translate API support was implemented. The `mainwindow.h` and `mainwindow.cpp` files represent an example of library usage with a GUI.

I used a JSON interface to access the APIs, therefore you can see how to parse JSON using Qt. I think there's a lot of room for improvement, so if you have any questions or suggestions, feel free to contact me about them!

Powered by Yandex.Translate
Powered by Yandex.Dictionary
