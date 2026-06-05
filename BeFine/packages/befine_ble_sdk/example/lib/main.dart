import 'package:flutter/material.dart';
import 'package:befine_ble_sdk/befine_ble_sdk.dart';

void main() => runApp(const SdkExampleApp());

class SdkExampleApp extends StatefulWidget {
  const SdkExampleApp({super.key});
  @override
  State<SdkExampleApp> createState() => _SdkExampleAppState();
}

class _SdkExampleAppState extends State<SdkExampleApp> {
  final _scanner = BleScanner();
  final _manager = BleManager(policy: const ReconnectPolicy());
  String _status = 'Idle';
  double _flow = 0.0;

  Future<void> _connect() async {
    setState(() => _status = 'Scanning...');
    final devices = await _scanner.scan(timeout: const Duration(seconds: 8));
    if (devices.isEmpty) {
      setState(() => _status = 'No BeFine device found');
      return;
    }
    setState(() => _status = 'Connecting...');
    await _manager.connect(devices.first);

    _manager.connectionStream.listen((s) {
      setState(() => _status = s.runtimeType.toString());
    });

    _manager.flowStream.listen((m) {
      setState(() => _flow = m.flowLpm);
    });

    await _manager.sendCommand(SessionCommand.start);
  }

  @override
  void dispose() {
    _manager.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('BeFine BLE SDK Example')),
        body: Center(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              Text('Status: $_status'),
              Text('Flow: ${_flow.toStringAsFixed(1)} L/min',
                  style: const TextStyle(fontSize: 32)),
              ElevatedButton(
                onPressed: _connect,
                child: const Text('Scan & Connect'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
