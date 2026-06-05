# BeFine — Diagrammes UML

Tous les diagrammes sont en **PlantUML** (`.puml`).  
Ils sont rendus automatiquement sur GitHub via l'extension PlantUML Preview ou via le serveur officiel.

## Fichiers

| Fichier | Diagramme | Description |
|---|---|---|
| `01_use_case.puml` | Use Case | Acteurs (Patient, Clinicien, Technicien) et 15 cas d'utilisation |
| `02_class_diagram.puml` | Classes | Shared Domain · befine_ble_sdk · App Domain · App Presentation |
| `03_sequence.puml` | Séquence | Session complète : scan BLE → inhalation → résultats → export |
| `04_state_machine.puml` | États | FSM firmware (C/FreeRTOS) + FSM Flutter (SessionBloc) en miroir |
| `05_activity.puml` | Activité | Flux patient complet avec swimlanes (Patient / App / SDK / Firmware) |

## Rendu local (VS Code)

1. Installer l'extension **PlantUML** (jebbs.plantuml)
2. Installer Java + Graphviz (`choco install graphviz` sur Windows)
3. Ouvrir un fichier `.puml` → `Alt+D` pour prévisualiser

## Rendu en ligne

Coller le contenu sur [https://www.plantuml.com/plantuml/uml/](https://www.plantuml.com/plantuml/uml/)

## Export PNG/SVG (CLI)

```bash
# Installer plantuml.jar
java -jar plantuml.jar -tsvg docs/uml/*.puml
java -jar plantuml.jar -tpng docs/uml/*.puml
```

Les images générées sont placées dans `docs/uml/` à côté des sources.
