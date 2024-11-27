import pandas as pd
import re

# Definiujemy nazwy, które chcemy przypisać do kolumn i wierszy
spreading_labels = ["SF5", "SF6", "SF7", "SF8", "SF9", "SF10", "SF11", "SF12"]
bandwidth_labels = ["BW_0200", "BW_0400", "BW_0800", "BW_1600"]

def parse_data(file_path):
    # Inicjalizacja pustej struktury służącej do przechowywania danych
    data = {}
    
    # Otwieranie i przetwarzanie pliku linia po linii
    with open(file_path, 'r') as file:
        for line in file:
            # Dopasowanie wzorca danych
            match = re.search(r"Spreading: (\d+) Bandwidth: (\d+) DataRate: ([\d.]+)", line)
            if match:
                spreading = int(match.group(1))
                bandwidth = int(match.group(2))
                datarate = float(match.group(3))

                # Dodanie datarate do odpowiedniej komórki
                if bandwidth not in data:
                    data[bandwidth] = {}
                data[bandwidth][spreading] = datarate

    # Konwersja słownika na DataFrame i przypisanie nazw kolumn i wierszy
    df = pd.DataFrame(data).T
    df.columns = spreading_labels[:df.shape[1]]
    df.index = bandwidth_labels[:df.shape[0]]
    df.index.name = 'Bandwidth'
    return df

# Ścieżka do pliku z danymi
file_path = 'dane_single100b.txt'
df = parse_data(file_path)
print("Tabela DataRate [bps] dla payload 100 bajtow:\n")
print(df)
file_path = 'dane_single200b.txt'
df = parse_data(file_path)
print("Tabela DataRate [bps] dla payload 200 bajtow:\n")
print(df)
file_path = 'dane_single229b.txt'
df = parse_data(file_path)
print("Tabela DataRate [bps] dla payload 229 bajtow:\n")
print(df)
file_path = 'dane_single230b.txt'
df = parse_data(file_path)
print("Tabela DataRate [bps] dla payload 230 bajtow:\n")
print(df)