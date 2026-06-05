import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';

class SettingsPage extends StatefulWidget {
  const SettingsPage({super.key});

  @override
  State<SettingsPage> createState() => _SettingsPageState();
}

class _SettingsPageState extends State<SettingsPage> {
  final _nameCtrl = TextEditingController();
  int _age = 30;
  bool _remindersEnabled = false;

  @override
  void initState() {
    super.initState();
    _loadPrefs();
  }

  Future<void> _loadPrefs() async {
    final prefs = await SharedPreferences.getInstance();
    setState(() {
      _nameCtrl.text = prefs.getString('patient_name') ?? '';
      _age = prefs.getInt('patient_age') ?? 30;
      _remindersEnabled = prefs.getBool('reminders') ?? false;
    });
  }

  Future<void> _savePrefs() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString('patient_name', _nameCtrl.text);
    await prefs.setInt('patient_age', _age);
    await prefs.setBool('reminders', _remindersEnabled);
    if (mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Paramètres sauvegardés')),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Réglages'),
        actions: [
          TextButton(
            onPressed: _savePrefs,
            child: const Text('Sauver',
                style: TextStyle(color: Colors.white)),
          ),
        ],
      ),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          Text('Profil patient',
              style: Theme.of(context).textTheme.titleLarge),
          const SizedBox(height: 12),
          TextField(
            controller: _nameCtrl,
            decoration: const InputDecoration(
              labelText: 'Nom',
              border: OutlineInputBorder(),
              prefixIcon: Icon(Icons.person_outline),
            ),
          ),
          const SizedBox(height: 12),
          Row(
            children: [
              const Text('Âge : '),
              Expanded(
                child: Slider(
                  value: _age.toDouble(),
                  min: 5,
                  max: 99,
                  divisions: 94,
                  label: '$_age ans',
                  onChanged: (v) => setState(() => _age = v.round()),
                ),
              ),
              Text('$_age ans'),
            ],
          ),
          const Divider(height: 32),
          Text('Rappels', style: Theme.of(context).textTheme.titleLarge),
          SwitchListTile(
            title: const Text('Activer les rappels de prise'),
            subtitle: const Text('Notification quotidienne'),
            value: _remindersEnabled,
            onChanged: (v) => setState(() => _remindersEnabled = v),
          ),
        ],
      ),
    );
  }

  @override
  void dispose() {
    _nameCtrl.dispose();
    super.dispose();
  }
}
