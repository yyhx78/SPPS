'use strict';

//const e = React.createElement;

class CustomContext extends React.Component {
    constructor(props) {
      super(props);

      this.contextRef = React.createRef();
      this.returnMenu = this.returnMenu.bind(this);

      this.state = {
        visible: false,
        x: 0,
        y: 0
      };
    }
  
    onContextMenu(event) {
      var self = this;
        //disable default context menu
        event.preventDefault();
        //update position after right click
        const clickX = event.clientX;
        const clickY = event.clientY;
        self.setState({
          visible: true,
          x: clickX,
          y: clickY
        });
    }

    onClick (event) {
      var self = this;
      if (self.contextRef.current && self.contextRef.current.id=='customcontext'){
        self.click(event.target.getAttribute('index'));
      }

      if (self.state.visible) {
        event.preventDefault();
        self.setState({
          visible: false,
          x: 0,
          y: 0
        });
      }
    }

    componentDidMount() {

      document.addEventListener('contextmenu', this.onContextMenu.bind(this));

      //hide after left click
      document.addEventListener('click', this.onClick.bind(this));
    }

    componentWillUnmount() {
      document.removeEventListener('contextmenu', this.onContextMenu);
      document.removeEventListener('click', this.onClick);
    }

    click(index) {
      if (!index)
        return;
      if(this.props.items[index].callback)
        this.props.items[index].callback(this.state.x, this.state.y);
      else{
        console.log("callback not registered for the menu item")
      }
    }

    returnMenu(items) {
      var myStyle = {
        'position': 'absolute',
        'zIndex': '9',
        'top': (this.state.y) + 'px',
        'left': (this.state.x + 2) + 'px'
      };
      
      return e("div", {
        className: "custom-context",
        id: "customcontext",
        style: myStyle,
        ref: this.contextRef
      }, items.map((item, index, arr) => {
        if (arr.length - 1 == index) {
          return e("div", {
            key: index,
            className: "custom-context-item-last",
            index:index
          }, item.label);
        } else {
          return e("div", {
            key: index,
            className: "custom-context-item",
            index:index
          }, item.label);
        }
      }));
    }
  
    render() {
        return e("div", {
          id: "cmenu"
        }, this.state.visible ? this.returnMenu(this.props.items) : null);
      }
}

var rightClickMenu = [
  {'label':'Center', 'callback': this.onSetViewCenter},
  {'label':'Reset', 'callback': this.onResetView}
];

function onSetViewCenter(clientX, clientY) {
  editor.signals.xhxProjectSettingsChanged.dispatch(3, {clientX:clientX, clientY:clientY});//2: reset view
}

function onResetView() {
  editor.signals.xhxProjectSettingsChanged.dispatch(2);//2: reset view
}


ReactDOM.render(e(CustomContext, {items: rightClickMenu}), document.querySelector('#divCustomContext'));
