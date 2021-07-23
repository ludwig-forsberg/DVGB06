/*
class ComponentTemplate extends HTMLElement{
    

    constructor(){
        super();
        this.loaded = false;
    }

    connectedCallback(){
        this.classList.add("template-component");

        this.attribute = GetAttribute(this, "attribute", "Default Value");
        this.innerHTML = ``;
        this.loaded = true;
    }

    static get observedAttributes(){
        return [];
    }
    attributeChangedCallback(attrName, oldVal, newVal) {  
		if (this.loaded && oldVal !== newVal) {
			
		}
	}

}
*/

function GetAttribute(object, attributeName, defaultValue){
    return (attributeName in object.attributes ? object.attributes[attributeName].value : defaultValue);
}

class InputComponent extends HTMLElement{
    
    constructor(){
        super();
        this.loaded = false;
        this._value = "";
    }

    get value(){
        if(this.loaded){
            return this.input.value;
        }
        return "";
    }

    set value(val){
        if(this.loaded){
            this.input.value = val;
        }
        return val;
    }

    connectedCallback(){
        this.type = GetAttribute(this, "type", "text");
        this.placeholder = GetAttribute(this, "placeholder", "");
        this.icon = GetAttribute(this, "icon", "");
        
        let value = this.innerHTML;

        this.innerHTML = `
        <div>
            <span></span>
            <input type="${this.type}" placeholder="${this.placeholder}"></input>
        </div>
        `;

        

        this.span = this.children[0].children[0];
        this.input = this.children[0].children[1];

        if(this.icon != ""){
            this.span.classList.add(`fas`);
            this.span.classList.add(`fa-${this.icon}`);
        }

        this.loaded = true;

        this.value = value;

        let _this = this;

        this.input.addEventListener('input', (event) => {
            if(_this.onValueChange != undefined){
                _this.onValueChange(_this.value);
            }
        });
    }

    static get observedAttributes(){
        return ["type", "placeholder", "icon"];
    }
    attributeChangedCallback(attrName, oldVal, newVal) {  
		if (this.loaded && oldVal !== newVal) {
			switch(attrName){
                case "type":
                    this.type = newVal;
                    this.input.setAttribute("type", newVal);
                    break;
                case "placeholder":
                    this.placeholder = newVal;
                    this.input.setAttribute("placeholder", newVal);
                    break;
                case "icon":
                    this.icon = newVal;
                    this.span.removeAttribute("class");
                    if(newVal != ""){
                        this.span.classList.add(`fas`);
                        this.span.classList.add(`fa-${newVal}`);
                    }
                    break;
            }
		}
	}

}


class ButtonComponent extends HTMLElement{
    
    constructor(){
        super();
        this.loaded = false;
    }

    connectedCallback(){
        this.text = GetAttribute(this, "text", "");
        this.icon = GetAttribute(this, "icon", "");

        if(this.innerHTML != ""){
            this.text = this.innerHTML;
        }

        this.innerHTML = `
        <div>
            <span></span>
            <p class="noselect">${this.text}</p>
        </div>
        `;

        
        this.div = this.children[0];
        this.span = this.div.children[0];
        this.p = this.div.children[1];

        if(this.icon != ""){
            this.span.classList.add(`fas`);
            this.span.classList.add(`fa-${this.icon}`);
        }

        this.loaded = true;
    }

    static get observedAttributes(){
        return ["text", "icon"];
    }
    attributeChangedCallback(attrName, oldVal, newVal) {  
		if (this.loaded && oldVal !== newVal) {
			switch(attrName){
                case "text":
                    this.text = newVal;
                    this.p.innerHTML = this.text;
                    break;
                case "icon":
                    this.icon = newVal;
                    this.span.removeAttribute("class");
                    if(newVal != ""){
                        this.span.classList.add(`fas`);
                        this.span.classList.add(`fa-${newVal}`);
                    }
                    break;
            }
		}
	}

}

class DropDownComponent extends HTMLElement{
    
    constructor(){
        super();
        this.loaded = false;
        this._selectedIndex = -1;
    }

    get selectedIndex (){
        return this._selectedIndex;
    }

    set selectedIndex(val){
        this._selectedIndex = val;
        this.setAttribute("selected-index", ""+val);
        return this.val;
    }

    get selectedElement(){
        if(this.selectedIndex != -1){
            return this.values[this.selectedIndex];
        }
        return undefined;
    }

    addValue(val){
        let index = this.values.length;

        let value = document.createElement("value");
        value.innerHTML = val;
        this.div.appendChild(value);

        value.classList.add("noselect");
        
        this.values.push(value);
        value.onclick = ()=>{
            this.selectedIndex = index;
            this.div.classList.remove("open");
        };

    }

    addElement(value){
        let index = this.values.length;

        
        this.div.appendChild(value);

        value.classList.add("noselect");
        
        this.values.push(value);
        value.onclick = ()=>{
            this.selectedIndex = index;
            this.div.classList.remove("open");
        };

    }

    removeValue(index){
        this.div.removeChild(this.values[index]);
        this.values.splice(index, 1);
        for(let i = 0; i < this.values.length; i++){
            let index = i;
            let value = this.values[index];
            value.onclick = ()=>{
                this.selectedIndex = index;
                this.div.classList.remove("open");
            };
        }
        if(index == this.selectedIndex){
            this.selectedIndex = -1;
        }else if(index < this.selectedIndex){
            this.selectedIndex--;
        }
        
    }

    clearValues(){
        this.values = [];
        this.div.innerHTML = "";
        this.selectedIndex = -1;
    }

    select(index){
        this.selectedIndex = index;
        this.div.classList.remove("open");
    }

    connectedCallback(){
        this.placeholder = GetAttribute(this, "placeholder", "Select a choice..");
        this.selectedIndex = parseInt(GetAttribute(this, "selected-index", "-1"));
        this.icon = GetAttribute(this, "icon", "");
        this.innerHTML = `
        <button-component class="noselect"></button-component>
        <div class="values">
            ${this.innerHTML}
        </div>
        `;

        this.button = this.children[0];
        this.div = this.children[1];

        this.values = [];
        for(let i = 0; i < this.div.children.length; i++){
            let index = i;
            let value = this.div.children[index];

            value.classList.add("noselect");
            this.values.push(value);
            value.onclick = ()=>{
                this.selectedIndex = index;
                this.div.classList.remove("open");
            };
        }

        this.button.setAttribute("icon", this.icon);

        if(this.selectedIndex == -1)
            this.button.setAttribute("text", this.placeholder);
        else
            this.button.setAttribute("text", this.values[this.selectedIndex].innerHTML);

        let _this = this;

        this.button.addEventListener("click", function (event) {
            if(_this.div.classList.contains("open")){
                _this.div.classList.remove("open");
            }else{
                _this.div.classList.add("open");
            }
        });

        window.addEventListener("click", function (event) {
            if (event.target != _this.button && event.target != _this.button.div) {
                _this.div.classList.remove("open");
            }
        });

        this.loaded = true;
    }

    static get observedAttributes(){
        return ["selected-index", "placeholder", "icon"];
    }
    attributeChangedCallback(attrName, oldVal, newVal) {  
		if (this.loaded && oldVal !== newVal) {
			switch(attrName){
                case "selected-index":
                    for(let i = 0; i < this.values.length; i++){
                        this.values[i].classList.remove("selected");
                    }
                    if(this.selectedIndex == -1)
                        this.button.setAttribute("text", this.placeholder);
                    else{
                        this.values[this.selectedIndex].classList.add("selected");
                        this.button.setAttribute("text", this.values[this.selectedIndex].innerHTML);
                    }
                    if(this.onSelectChange != undefined){
                        this.onSelectChange(this.selectedIndex);
                    }
                    break;
                case "placeholder":
                    this.placeholder = newVal;
                    if(this.selectedIndex == -1)
                        this.button.innerHTML = this.placeholder;
                    break;
                case "icon":
                    this.icon = newVal;
                    this.button.setAttribute("icon", newVal);
                    break;
            }
		}
	}

}

// class TableComponent extends HTMLTableElement{
    

//     constructor(){
//         super();
//         this.loaded = false;
//     }

//     connectedCallback(){
//         this.title = GetAttribute(this, "title", "");
//         console.log("" + this.innerHTML)
//         /*this.innerHTML = `
//             <h1>${this.title}</h1>
//             <table>${this.innerHTML}</table>
//         `;*/
//         this.loaded = true;
//     }

//     static get observedAttributes(){
//         return [];
//     }
//     attributeChangedCallback(attrName, oldVal, newVal) {  
// 		if (this.loaded && oldVal !== newVal) {
			
// 		}
// 	}

// }

window.addEventListener('load', (event) => {
    customElements.define('input-component', InputComponent);
    customElements.define('button-component', ButtonComponent);
    customElements.define('drop-down-component', DropDownComponent);
    // customElements.define('table-component', TableComponent, {extends: "table"});
});