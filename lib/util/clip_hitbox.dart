import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

class ClipHitbox extends SingleChildRenderObjectWidget {
  /// Creates a rectangular clip.
  ///
  /// If [clipper] is null, the clip will match the layout size and position of
  /// the child.
  const ClipHitbox({Key? key, this.clipper, Widget? child}) : super(key: key, child: child);

  /// If non-null, determines which clip to use.
  final CustomClipper<Rect>? clipper;

  @override
  RenderClipHitbox createRenderObject(BuildContext context) {
    return RenderClipHitbox(clipper: clipper);
  }

  @override
  void updateRenderObject(BuildContext context, RenderClipHitbox renderObject) {
    renderObject.clipper = clipper;
  }

  @override
  void didUnmountRenderObject(RenderClipHitbox renderObject) {
    renderObject.clipper = null;
  }

  @override
  void debugFillProperties(DiagnosticPropertiesBuilder properties) {
    super.debugFillProperties(properties);
    properties.add(DiagnosticsProperty<CustomClipper<Rect>>('clipper', clipper, defaultValue: null));
  }
}

class RenderClipHitbox extends _RenderCustomClip<Rect> {
  /// Creates a rectangular clip.
  ///
  /// If [clipper] is null, the clip will match the layout size and position of
  /// the child.
  RenderClipHitbox({
    RenderBox? child,
    CustomClipper<Rect>? clipper,
  }) : super(child: child, clipper: clipper);

  @override
  Rect get _defaultClip => Offset.zero & size;

  @override
  bool hitTest(BoxHitTestResult result, {required Offset position}) {
    if (_clipper != null) {
      _updateClip();
      assert(_clip != null);
      if (!_clip!.contains(position)) return false;
    }
    return super.hitTest(result, position: position);
  }
}

abstract class _RenderCustomClip<T> extends RenderProxyBox {
  _RenderCustomClip({
    RenderBox? child,
    CustomClipper<T>? clipper,
  })  : _clipper = clipper,
        super(child);

  /// If non-null, determines which clip to use on the child.
  CustomClipper<T>? get clipper => _clipper;
  CustomClipper<T>? _clipper;

  set clipper(CustomClipper<T>? newClipper) {
    if (_clipper == newClipper) return;
    final CustomClipper<T>? oldClipper = _clipper;
    _clipper = newClipper;
    assert(newClipper != null || oldClipper != null);
    if (newClipper == null ||
        oldClipper == null ||
        newClipper.runtimeType != oldClipper.runtimeType ||
        newClipper.shouldReclip(oldClipper)) {
      _markNeedsClip();
    }
    if (attached) {
      oldClipper?.removeListener(_markNeedsClip);
      newClipper?.addListener(_markNeedsClip);
    }
  }

  @override
  void attach(PipelineOwner owner) {
    super.attach(owner);
    _clipper?.addListener(_markNeedsClip);
  }

  @override
  void detach() {
    _clipper?.removeListener(_markNeedsClip);
    super.detach();
  }

  void _markNeedsClip() {
    _clip = null;
    markNeedsPaint();
    markNeedsSemanticsUpdate();
  }

  T get _defaultClip;

  T? _clip;

  @override
  void performLayout() {
    final Size? oldSize = hasSize ? size : null;
    super.performLayout();
    if (oldSize != size) _clip = null;
  }

  void _updateClip() {
    _clip ??= _clipper?.getClip(size) ?? _defaultClip;
  }

  @override
  Rect describeApproximatePaintClip(RenderObject child) {
    return _clipper?.getApproximateClipRect(size) ?? Offset.zero & size;
  }
}
