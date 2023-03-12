// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target, unnecessary_question_mark

part of 'window_stack_provider.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$WindowStack {
  List<int> get stack => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $WindowStackCopyWith<WindowStack> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $WindowStackCopyWith<$Res> {
  factory $WindowStackCopyWith(
          WindowStack value, $Res Function(WindowStack) then) =
      _$WindowStackCopyWithImpl<$Res, WindowStack>;
  @useResult
  $Res call({List<int> stack});
}

/// @nodoc
class _$WindowStackCopyWithImpl<$Res, $Val extends WindowStack>
    implements $WindowStackCopyWith<$Res> {
  _$WindowStackCopyWithImpl(this._value, this._then);

  // ignore: unused_field
  final $Val _value;
  // ignore: unused_field
  final $Res Function($Val) _then;

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? stack = null,
  }) {
    return _then(_value.copyWith(
      stack: null == stack
          ? _value.stack
          : stack // ignore: cast_nullable_to_non_nullable
              as List<int>,
    ) as $Val);
  }
}

/// @nodoc
abstract class _$$_WindowStackCopyWith<$Res>
    implements $WindowStackCopyWith<$Res> {
  factory _$$_WindowStackCopyWith(
          _$_WindowStack value, $Res Function(_$_WindowStack) then) =
      __$$_WindowStackCopyWithImpl<$Res>;
  @override
  @useResult
  $Res call({List<int> stack});
}

/// @nodoc
class __$$_WindowStackCopyWithImpl<$Res>
    extends _$WindowStackCopyWithImpl<$Res, _$_WindowStack>
    implements _$$_WindowStackCopyWith<$Res> {
  __$$_WindowStackCopyWithImpl(
      _$_WindowStack _value, $Res Function(_$_WindowStack) _then)
      : super(_value, _then);

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? stack = null,
  }) {
    return _then(_$_WindowStack(
      stack: null == stack
          ? _value._stack
          : stack // ignore: cast_nullable_to_non_nullable
              as List<int>,
    ));
  }
}

/// @nodoc

class _$_WindowStack extends _WindowStack {
  const _$_WindowStack({required final List<int> stack})
      : _stack = stack,
        super._();

  final List<int> _stack;
  @override
  List<int> get stack {
    if (_stack is EqualUnmodifiableListView) return _stack;
    // ignore: implicit_dynamic_type
    return EqualUnmodifiableListView(_stack);
  }

  @override
  String toString() {
    return 'WindowStack(stack: $stack)';
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_WindowStack &&
            const DeepCollectionEquality().equals(other._stack, _stack));
  }

  @override
  int get hashCode =>
      Object.hash(runtimeType, const DeepCollectionEquality().hash(_stack));

  @JsonKey(ignore: true)
  @override
  @pragma('vm:prefer-inline')
  _$$_WindowStackCopyWith<_$_WindowStack> get copyWith =>
      __$$_WindowStackCopyWithImpl<_$_WindowStack>(this, _$identity);
}

abstract class _WindowStack extends WindowStack {
  const factory _WindowStack({required final List<int> stack}) = _$_WindowStack;
  const _WindowStack._() : super._();

  @override
  List<int> get stack;
  @override
  @JsonKey(ignore: true)
  _$$_WindowStackCopyWith<_$_WindowStack> get copyWith =>
      throw _privateConstructorUsedError;
}
