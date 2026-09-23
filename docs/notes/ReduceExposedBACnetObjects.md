Yes, reducing objects would likely improve MS/TP behavior, but it would not fix the whole reboot/COV-state problem by itself.

The other project worked better mainly because it had a much lighter MS/TP workload:

- `ESP32-BACnet-Master` had about `9` Analog Inputs.
- This LoRa gateway has `32` Analog Inputs, plus the other AV/BV/BI/BO objects.
- The NAE likely subscribes to many or all points.
- More objects means more `ReadPropertyMultiple`, more `SubscribeCOVProperty`, more COV checks, and more COV notifications after LoRa updates.
- MS/TP can only send when this device owns the token, and only up to `Max_Info_Frames` per token turn.

So with fewer objects, the old project had fewer NAE subscription requests, fewer notifications, and fewer chances for the one-frame/limited-queue MS/TP path to get congested. That is why it could appear stable even with weaker MS/TP buffering/timing.

Reducing objects here would help in these ways:

- Faster NAE rediscovery after reboot.
- Fewer COV subscriptions to recreate.
- Less MS/TP traffic after LoRa updates.
- Lower chance of delayed or dropped COV ACK/notification traffic.
- Less time before the NAE has read all object metadata and values.

But reducing objects would not solve this specific mismatch:

- On reboot, the device loses RAM COV subscriptions.
- The NAE may not mark it offline quickly.
- If the NAE thinks subscriptions still exist, it may not immediately resubscribe.
- Then object updates may still not push to the NAE, even with fewer objects.

So the answer is:

Reducing objects would improve timing and reliability on MS/TP, especially after reboot, but it is a mitigation, not the root fix. The root issue is that this device has a much heavier object/COV load than the old project, and MS/TP plus RAM-only COV subscriptions make reboot recovery fragile.

A practical middle ground would be to reduce the exposed BACnet points to only the ones the NAE truly needs. For example, if only 6 LoRa sensors are active, expose only those 24 AI values instead of all 32, and avoid unused AV/BV/BI/BO objects if the NAE does not need them. That would make the NAE side recover faster and put less pressure on the MS/TP bus.