# 🧊 Bloxorz 3D - OpenGL Implementation

![OpenGL](https://img.shields.io/badge/Graphics-OpenGL%203.3-blue?style=for-the-badge&logo=opengl)
![C++](https://img.shields.io/badge/Language-C%2B%2B-green?style=for-the-badge&logo=c%2B%2B)

O reinterpretare modernă a jocului clasic de puzzle **Bloxorz**, dezvoltată ca proiect pentru disciplina Grafică pe Calculator. Proiectul transpune mecanica originală de tip *tile-based* într-un mediu 3D complet, utilizând tehnici avansate de randare și simulare matematică.

---

## 🚀 Demo

---

## ✨ Originalitate & Adaptare
Proiectul nu este o simplă clonă, ci o modernizare a jocului original:
1. **Modificarea Atmosferei:** Trecerea de la designul minimalist original la o estetică modernă, cosmică.
2. **Fizică Vizuală Procedurală:** Animații specifice pentru succes (plutire cu auto-rotație) și eșec (cădere liberă accelerată gravitațional cu rotație haotică pe axa de mișcare).
3. **Feedback Vizual prin Umbre:** Utilizarea umbrelor pentru a indica precis zona de contact cu platforma, un element critic de gameplay absent în versiunile originale 2D.

---

## 🛠️ Stack Tehnologic
* **Grafică:** OpenGL 3.3+ (Core Profile)
* **Biblioteci:** * `GLEW` - Managementul extensiilor
  * `FreeGLUT` - Windowing & Input
  * `GLM` - Matematică vectorială și matricială
  * `stb_image` - Încărcare texturi (JPG/PNG)

---

## 🎮 Control & Gameplay
* **W / A / S / D** - Rostogolirea blocului.
* **Obiectiv:** Poziționează blocul vertical pe celula țintă pentru a avansa la nivelul următor.
* **Provocare:** Platforma este suspendată; orice mișcare greșită care scoate centrul de masă în afara celulelor active duce la pierderea nivelului.

---

## 📁 Structură Fișiere
* `main.cpp` - Nucleul jocului (Loop, State Machine, Input).
* `example.vert/frag` - Shaders principali pentru iluminare și textură.
* `shadow.vert/frag` - Shaders pentru generarea hărții de umbre.
* `skybox.vert/frag` - Shaders pentru randarea fundalului.
