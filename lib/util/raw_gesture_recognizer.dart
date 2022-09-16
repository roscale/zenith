import 'package:flutter/gestures.dart';
import 'package:flutter/rendering.dart';

/// Gesture recognizer that provides similar callbacks to the Listener widget, but fights in
/// gesture arenas against other GestureDetectors.
/// onPointerCancel is called when it loses in the arena.
class RawGestureRecognizer extends GestureRecognizer {
  RawGestureRecognizer({
    Object? debugOwner,
    @Deprecated(
      'Migrate to supportedDevices. '
      'This feature was deprecated after v2.3.0-1.0.pre.',
    )
        PointerDeviceKind? kind,
    Set<PointerDeviceKind>? supportedDevices,
  }) : super(
          debugOwner: debugOwner,
          kind: kind,
          supportedDevices: supportedDevices,
        );

  PointerDownEventListener? onPointerDown;
  PointerMoveEventListener? onPointerMove;
  PointerUpEventListener? onPointerUp;
  PointerCancelEventListener? onPointerCancel;

  Map<int, _MultiDragPointerState>? _pointers = <int, _MultiDragPointerState>{};

  @override
  void addAllowedPointer(PointerDownEvent event) {
    assert(_pointers != null);
    assert(!_pointers!.containsKey(event.pointer));
    final GestureArenaEntry arenaEntry = GestureBinding.instance.gestureArena.add(event.pointer, this);
    final _MultiDragPointerState state = _createNewPointerState(event, arenaEntry);
    _pointers![event.pointer] = state;
    GestureBinding.instance.pointerRouter.addRoute(event.pointer, _handleEvent);
  }

  _MultiDragPointerState _createNewPointerState(PointerDownEvent event, GestureArenaEntry entry) {
    return _MultiDragPointerState(downEvent: event, arenaEntry: entry);
  }

  void _handleEvent(PointerEvent event) {
    assert(_pointers != null);
    assert(_pointers!.containsKey(event.pointer));
    final _MultiDragPointerState state = _pointers![event.pointer]!;

    if (event is PointerDownEvent && onPointerDown != null) {
      invokeCallback<void>('onPointerDown', () {
        onPointerDown!(event.copyWith(
          position: state.localPosition,
        ));
      });
    } else if (event is PointerMoveEvent && onPointerMove != null) {
      if (state.gestureAccepted) {
        final accumulatedDelta = event.localDelta + state.pendingDelta;
        state.localPosition += accumulatedDelta;

        var moveEvent = event.copyWith(
          position: state.localPosition,
          delta: accumulatedDelta,
        );
        state.pendingDelta = Offset.zero;
        invokeCallback<void>('onPointerMove', () => onPointerMove!(moveEvent));
      } else {
        state.pendingDelta += event.delta;
      }
    } else if (event is PointerUpEvent && onPointerUp != null) {
      invokeCallback<void>('onPointerUp', () {
        onPointerUp!(event.copyWith(
          position: state.localPosition,
        ));
      });
      state.resolve(GestureDisposition.accepted);
      _removeState(event.pointer);
    } else if (event is PointerCancelEvent && onPointerCancel != null) {
      state.resolve(GestureDisposition.rejected);
    }
  }

  @override
  void acceptGesture(int pointer) {
    assert(_pointers != null);
    _pointers![pointer]!.gestureAccepted = true;
  }

  @override
  void rejectGesture(int pointer) {
    assert(_pointers != null);
    if (_pointers!.containsKey(pointer)) {
      var state = _pointers![pointer]!;
      invokeCallback<void>('onPointerCancel', () {
        onPointerCancel!(PointerCancelEvent(
          pointer: pointer,
          kind: state.downEvent.kind,
          device: state.downEvent.device,
          embedderId: state.downEvent.embedderId,
        ));
      });
      _removeState(pointer);
    }
  }

  void _removeState(int pointer) {
    assert(_pointers != null);
    assert(_pointers!.containsKey(pointer));
    GestureBinding.instance.pointerRouter.removeRoute(pointer, _handleEvent);
    _pointers!.remove(pointer)!.dispose();
  }

  @override
  void dispose() {
    _pointers!.keys.toList().forEach((int pointer) {
      _pointers![pointer]!.arenaEntry!.resolve(GestureDisposition.rejected);
    });
    assert(_pointers!.isEmpty);
    _pointers = null;
    super.dispose();
  }

  @override
  String get debugDescription => "Raw gesture recognizer";
}

class _MultiDragPointerState {
  /// Creates per-pointer state for a [RawGestureRecognizer].
  _MultiDragPointerState({required this.downEvent, required this.arenaEntry});

  final PointerDownEvent downEvent;
  late Offset localPosition = downEvent.localPosition;
  GestureArenaEntry? arenaEntry;

  bool gestureAccepted = false;
  Offset pendingDelta = Offset.zero;

  /// Resolve this pointer's entry in the [GestureArenaManager] with the given disposition.
  void resolve(GestureDisposition disposition) {
    arenaEntry!.resolve(disposition);
  }

  /// Releases any resources used by the object.
  void dispose() {
    arenaEntry = null;
  }
}
