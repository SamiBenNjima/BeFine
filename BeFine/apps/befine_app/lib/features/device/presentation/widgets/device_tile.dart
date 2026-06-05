import 'package:flutter/material.dart';
import '../../domain/entities/device.dart';

class DeviceTile extends StatelessWidget {
  const DeviceTile({super.key, required this.device, required this.onTap});

  final Device device;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: ListTile(
        leading: const Icon(Icons.bluetooth, color: Colors.blue),
        title: Text(device.name),
        subtitle: Text(device.id),
        trailing: _RssiIndicator(rssi: device.rssi),
        onTap: onTap,
      ),
    );
  }
}

class _RssiIndicator extends StatelessWidget {
  const _RssiIndicator({required this.rssi});
  final int rssi;

  @override
  Widget build(BuildContext context) {
    final bars = rssi > -60 ? 4 : rssi > -75 ? 3 : rssi > -85 ? 2 : 1;
    return Row(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.end,
      children: List.generate(4, (i) {
        final active = i < bars;
        return Container(
          width: 5,
          height: 6.0 + i * 4,
          margin: const EdgeInsets.symmetric(horizontal: 1),
          decoration: BoxDecoration(
            color: active ? Colors.blue : Colors.grey.shade300,
            borderRadius: BorderRadius.circular(2),
          ),
        );
      }),
    );
  }
}
