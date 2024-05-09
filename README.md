# Assignment from Operating Systems (ELTE FI)

Final assignment from Operating Systems taught at the Faculty of Informatics at ELTE.

## Task

*(The task was originally assigned in Hungarian. Translation coming soon...)*

### Part 1

Tavasz van, közeledik a Húsvét. Nyuszi mama is készülődik, mégpedig a kis nyuszi fiúknak gyűjt locsoló versikéket. Ilyen gyöngyszemeket mint például: "Piros tojás zöld-fehér nyuszi, locsolásér jár két puszi!" Persze akad aki akár ezt is, kicsit másképp tudja. Minden módosított változatot amit hall, feljegyez.

Készítsen C nyelvű alkalmazást, ami segít Nyuszi mamának a locsoló versikék feljegyzésében (új verset tudjunk felvenni), tudjuk listáztatni a már ismert verseket, tudjunk törölni ha szükséges és tudjunk módosítani egy meglévő verset. 

A locsoló verseket fájlban tároljuk.

Készítsen C  nyelvű programot ami  ezt a feladatot megoldja, a megoldásnak vagy az opsys.inf.elte.hu kiszolgálón, vagy egy hozzá hasonló Linux rendszeren kell futnia. A megoldást a beadási határidőt követő héten be kell mutatni a gyakorlatvezetőnek.

### Part 2

Tavasz van, közeledik a Húsvét. Nyuszi mama is készült, mégpedig a kis nyuszi fiúknak gyűjtött locsoló versikéket. A Nyuszi Húsvét javában tart, a fiúk locsolni készülnek.

Nyuszi mama (szülő) a locsoló versikéket fájlban tárolja. Miközben ha új verset hall, azt továbbra is feljegyzi, esetleg módosítja, de a 4 nyuszi kisfia közül kiválasztja (véletlenszám) a legrátermettebbet (gyerek) és elküldi locsolni a Barátfai nyuszi család lánytagjainak locsolására. Ez a Locsolás lehetőség jelenjen meg új menüpontként a korábbiak mellett. Mikor a nyuszi fiú megérkezik Barátfára küld egy jelzést a mamának, aki erre válaszul 2 versikét is küld csövön keresztül a fiúnak. Ezeket a verseket a fiú képernyőre írja, majd választ egyet (véletlenszám). A kiválasztott verset üzenetsoron visszaküldi a mamának. (Később ezt a verset már ne használják fel.) Ezután nyuszi fiú elmondja Barátfán a verset (képernyőre írja, majd hozzá teszi: Szabad-e locsolni! Majd meglocsolja a lányokat és hazatér Nyuszi mamához. (terminál))

Készítsen C  nyelvű programot ami  ezt a feladatot megoldja, a megoldásnak vagy az opsys.inf.elte.hu kiszolgálón, vagy egy hozzá hasonló Linux rendszeren kell futnia. A megoldást a beadási határidőt követő héten be kell mutatni a gyakorlatvezetőnek.

## Compilation

To compile the source code, simply run the `Makefile` the following way.

```shell
make
```

The compiled program can be executed via the following name.

```shell
./bunny
```

Please note that the path of the `poems.txt` file is hard-coded, so if the executable is moved or copied, make sure to do so alongside the text file.

## Remarks

- Originally, the requirements DISCOURAGED us to use header files and NOT to modularise our code. As the due dates are over, I decided to refactor the code base so that it be clearer to see and evaluate each component separately.
- In the header files, I used macros containing lower-case letters. While this is not a standard solution (it certainly goes against conventional C style-guides), I chose it to reduce lengthy header file names.